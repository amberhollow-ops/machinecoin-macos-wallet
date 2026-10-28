// Ephemeral BIP70 certificates for tests only. No fixture private keys are saved.
#ifndef MACHINECOIN_QT_TEST_PAYMENTCERTFIXTURE_H
#define MACHINECOIN_QT_TEST_PAYMENTCERTFIXTURE_H

#include <qt/paymentrequest.pb.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509v3.h>
#include <memory>
#include <stdexcept>
#include <vector>

class PaymentCertFixture {
    using Key = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
    using Cert = std::unique_ptr<X509, decltype(&X509_free)>;
    static void Require(bool ok) { if (!ok) throw std::runtime_error("Cannot create payment certificate fixture"); }
    static Key MakeKey() {
        std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr), EVP_PKEY_CTX_free);
        Require(bool(ctx));
        Require(EVP_PKEY_keygen_init(ctx.get()) == 1);
        Require(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), 2048) == 1);
        EVP_PKEY* key = nullptr;
        Require(EVP_PKEY_keygen(ctx.get(), &key) == 1);
        return Key(key, EVP_PKEY_free);
    }
    static Cert MakeCert(const std::string& name, EVP_PKEY* key, X509* issuer, EVP_PKEY* signer, int serial, bool ca, long start, long end) {
        Cert cert(X509_new(), X509_free);
        Require(bool(cert));
        Require(X509_set_version(cert.get(), 2) == 1);
        Require(ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), serial) == 1);
        Require(X509_gmtime_adj(X509_getm_notBefore(cert.get()), start) != nullptr);
        Require(X509_gmtime_adj(X509_getm_notAfter(cert.get()), end) != nullptr);
        Require(X509_set_pubkey(cert.get(), key) == 1);
        auto subject = X509_get_subject_name(cert.get());
        Require(X509_NAME_add_entry_by_txt(subject, "CN", MBSTRING_ASC,
            reinterpret_cast<const unsigned char*>(name.c_str()), -1, -1, 0) == 1);
        Require(X509_set_issuer_name(cert.get(), issuer ? X509_get_subject_name(issuer) : subject) == 1);
        auto addExtension = [&](int nid, const char* value) {
            std::unique_ptr<X509_EXTENSION, decltype(&X509_EXTENSION_free)> ext(
                X509V3_EXT_conf_nid(nullptr, nullptr, nid, const_cast<char*>(value)), X509_EXTENSION_free);
            Require(bool(ext));
            Require(X509_add_ext(cert.get(), ext.get(), -1) == 1);
        };
        addExtension(NID_basic_constraints, ca ? "critical,CA:TRUE" : "critical,CA:FALSE");
        addExtension(NID_key_usage, ca ? "critical,keyCertSign,cRLSign" : "critical,digitalSignature");
        Require(X509_sign(cert.get(), signer, EVP_sha256()) > 0);
        return cert;
    }
    Key caKey{MakeKey()};
    Key merchantKey{MakeKey()};
    Cert root{MakeCert("Runtime payment test CA", caKey.get(), nullptr, caKey.get(), 1, true, -86400, 86400)};
public:
    X509* Root() const { return root.get(); }
    std::vector<unsigned char> Request(const std::vector<unsigned char>& templateData, int intermediates = 0,
        bool expiredLeaf = false, bool expiredIntermediate = false, bool futureLeaf = false, bool badSignature = false) {
        std::vector<Cert> chain;
        X509* issuer = root.get();
        for (int i = 0; i < intermediates; ++i) {
            chain.push_back(MakeCert("Intermediate " + std::to_string(i), caKey.get(), issuer, caKey.get(), i + 2,
                true, -172800, expiredIntermediate && i == 0 ? -86400 : 86400));
            issuer = chain.back().get();
        }
        chain.push_back(MakeCert(intermediates ? "testmerchant8.org" : "testmerchant.org", merchantKey.get(), issuer,
            caKey.get(), 100, false, futureLeaf ? 86400 : -172800, expiredLeaf ? -86400 : 172800));
        payments::X509Certificates certificates;
        auto append = [&](X509* cert) {
            int size = i2d_X509(cert, nullptr);
            Require(size > 0);
            std::string der(size, '\0');
            auto out = reinterpret_cast<unsigned char*>(&der[0]);
            Require(i2d_X509(cert, &out) == size);
            certificates.add_certificate(der);
        };
        for (auto it = chain.rbegin(); it != chain.rend(); ++it) append(it->get());
        append(root.get());
        payments::PaymentRequest request;
        Require(request.ParseFromArray(templateData.data(), templateData.size()));
        request.set_pki_type("x509+sha256");
        request.set_pki_data(certificates.SerializeAsString());
        request.set_signature("");
        const std::string payload = request.SerializeAsString();
        std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
        Require(bool(ctx));
        Require(EVP_DigestSignInit(ctx.get(), nullptr, EVP_sha256(), nullptr, merchantKey.get()) == 1);
        Require(EVP_DigestSignUpdate(ctx.get(), payload.data(), payload.size()) == 1);
        size_t length = 0;
        Require(EVP_DigestSignFinal(ctx.get(), nullptr, &length) == 1);
        std::string signature(length, '\0');
        Require(EVP_DigestSignFinal(ctx.get(), reinterpret_cast<unsigned char*>(&signature[0]), &length) == 1);
        signature.resize(length);
        if (badSignature) signature[0] ^= 1;
        request.set_signature(signature);
        const std::string serialized = request.SerializeAsString();
        return {serialized.begin(), serialized.end()};
    }
};
#endif
