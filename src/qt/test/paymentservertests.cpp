// Copyright (c) 2009-2017 The Machinecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/test/paymentservertests.h>

#include <qt/optionsmodel.h>
#include <qt/test/paymentrequestdata.h>
#include <qt/test/paymentcertfixture.h>

#include <amount.h>
#include <chainparams.h>
#include <interfaces/node.h>
#include <random.h>
#include <script/script.h>
#include <script/standard.h>
#include <util.h>
#include <utilstrencodings.h>

#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include <QFileOpenEvent>
#include <QTemporaryFile>

//
// Test payment request handling
//

static SendCoinsRecipient handleRequest(PaymentServer* server, std::vector<unsigned char>& data)
{
    RecipientCatcher sigCatcher;
    QObject::connect(server, SIGNAL(receivedPaymentRequest(SendCoinsRecipient)),
        &sigCatcher, SLOT(getRecipient(SendCoinsRecipient)));

    // Write data to a temp file:
    QTemporaryFile f;
    f.open();
    f.write((const char*)data.data(), data.size());
    f.close();

    // Create a QObject, install event filter from PaymentServer
    // and send a file open event to the object
    QObject object;
    object.installEventFilter(server);
    QFileOpenEvent event(f.fileName());
    // If sending the event fails, this will cause sigCatcher to be empty,
    // which will lead to a test failure anyway.
    QCoreApplication::sendEvent(&object, &event);

    QObject::disconnect(server, SIGNAL(receivedPaymentRequest(SendCoinsRecipient)),
        &sigCatcher, SLOT(getRecipient(SendCoinsRecipient)));

    // Return results from sigCatcher
    return sigCatcher.recipient;
}

void PaymentServerTests::paymentServerTests()
{
    SelectParams(CBaseChainParams::MAIN);
    auto node = interfaces::MakeNode();
    OptionsModel optionsModel(*node);
    PaymentCertFixture certificates;
    PaymentCertFixture untrustedCertificates;
    const auto templateData = DecodeBase64(paymentrequest1_cert1_BASE64);
    std::unique_ptr<PaymentServer> server(new PaymentServer(nullptr, false));
    X509_STORE* caStore = X509_STORE_new();
    QVERIFY(X509_STORE_add_cert(caStore, certificates.Root()) == 1);
    PaymentServer::LoadRootCAs(caStore);
    server->setOptionsModel(&optionsModel);
    server->uiReady();

    std::vector<unsigned char> data;
    SendCoinsRecipient r;
    QString merchant;

    // Now feed PaymentRequests to server, and observe signals it produces

    // This payment request validates directly against the
    // generated test certificate authority:
    data = certificates.Request(templateData);
    r = handleRequest(server.get(), data);
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString("testmerchant.org"));

    // Signed, but expired, merchant cert in the request:
    data = certificates.Request(templateData, 0, true);
    r = handleRequest(server.get(), data);
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString(""));

    // 10-long certificate chain, all intermediates valid:
    data = certificates.Request(templateData, 8);
    r = handleRequest(server.get(), data);
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString("testmerchant8.org"));

    // Long certificate chain, with an expired certificate in the middle:
    data = certificates.Request(templateData, 8, false, true);
    r = handleRequest(server.get(), data);
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString(""));

    // Validly signed, but by a CA not in our root CA list:
    data = untrustedCertificates.Request(templateData);
    r = handleRequest(server.get(), data);
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString(""));

    // Reject a not-yet-valid certificate and a modified payment signature too.
    data = certificates.Request(templateData, 0, false, false, true);
    r = handleRequest(server.get(), data);
    QVERIFY(!r.paymentRequest.getMerchant(caStore, merchant));
    QVERIFY(merchant.isEmpty());
    data = certificates.Request(templateData, 0, false, false, false, true);
    r = handleRequest(server.get(), data);
    QVERIFY(!r.paymentRequest.getMerchant(caStore, merchant));
    QVERIFY(merchant.isEmpty());

    // Try again with no root CA's, verifiedMerchant should be empty:
    caStore = X509_STORE_new();
    PaymentServer::LoadRootCAs(caStore);
    data = certificates.Request(templateData);
    r = handleRequest(server.get(), data);
    r.paymentRequest.getMerchant(caStore, merchant);
    QCOMPARE(merchant, QString(""));


    QByteArray byteArray;

    // For the tests below we just need the payment request data from
    // paymentrequestdata.h parsed + stored in r.paymentRequest.
    //
    // These tests require us to bypass the following normal client execution flow
    // shown below to be able to explicitly just trigger a certain condition!
    //
    // handleRequest()
    // -> PaymentServer::eventFilter()
    //   -> PaymentServer::handleURIOrFile()
    //     -> PaymentServer::readPaymentRequestFromFile()
    //       -> PaymentServer::processPaymentRequest()

    // Contains a testnet paytoaddress, so payment request network doesn't match client network:
    data = DecodeBase64(paymentrequest1_cert2_BASE64);
    byteArray = QByteArray((const char*)data.data(), data.size());
    r.paymentRequest.parse(byteArray);
    // Ensure the request is initialized, because network "main" is default, even for
    // uninitialized payment requests and that will fail our test here.
    QVERIFY(r.paymentRequest.IsInitialized());
    QCOMPARE(PaymentServer::verifyNetwork(*node, r.paymentRequest.getDetails()), false);

    // Expired payment request (expires is set to 1 = 1970-01-01 00:00:01):
    data = DecodeBase64(paymentrequest2_cert2_BASE64);
    byteArray = QByteArray((const char*)data.data(), data.size());
    r.paymentRequest.parse(byteArray);
    // Ensure the request is initialized
    QVERIFY(r.paymentRequest.IsInitialized());
    // compares 1 < GetTime() == false (treated as expired payment request)
    QCOMPARE(PaymentServer::verifyExpired(r.paymentRequest.getDetails()), true);

    // Unexpired payment request (expires is set to 0x7FFFFFFFFFFFFFFF = max. int64_t):
    // 9223372036854775807 (uint64), 9223372036854775807 (int64_t) and -1 (int32_t)
    // -1 is 1969-12-31 23:59:59 (for a 32 bit time values)
    data = DecodeBase64(paymentrequest3_cert2_BASE64);
    byteArray = QByteArray((const char*)data.data(), data.size());
    r.paymentRequest.parse(byteArray);
    // Ensure the request is initialized
    QVERIFY(r.paymentRequest.IsInitialized());
    // compares 9223372036854775807 < GetTime() == false (treated as unexpired payment request)
    QCOMPARE(PaymentServer::verifyExpired(r.paymentRequest.getDetails()), false);

    // Unexpired payment request (expires is set to 0x8000000000000000 > max. int64_t, allowed uint64):
    // 9223372036854775808 (uint64), -9223372036854775808 (int64_t) and 0 (int32_t)
    // 0 is 1970-01-01 00:00:00 (for a 32 bit time values)
    data = DecodeBase64(paymentrequest4_cert2_BASE64);
    byteArray = QByteArray((const char*)data.data(), data.size());
    r.paymentRequest.parse(byteArray);
    // Ensure the request is initialized
    QVERIFY(r.paymentRequest.IsInitialized());
    // compares -9223372036854775808 < GetTime() == true (treated as expired payment request)
    QCOMPARE(PaymentServer::verifyExpired(r.paymentRequest.getDetails()), true);

    // Test BIP70 DoS protection:
    unsigned char randData[BIP70_MAX_PAYMENTREQUEST_SIZE + 1];
    GetRandBytes(randData, sizeof(randData));
    // Write data to a temp file:
    QTemporaryFile tempFile;
    tempFile.open();
    tempFile.write((const char*)randData, sizeof(randData));
    tempFile.close();
    // compares 50001 <= BIP70_MAX_PAYMENTREQUEST_SIZE == false
    QCOMPARE(PaymentServer::verifySize(tempFile.size()), false);

    // Use Machinecoin's actual limit, rather than Bitcoin's 21-million fixture.
    data = DecodeBase64(paymentrequest5_cert2_BASE64);
    payments::PaymentRequest overflowRequest;
    QVERIFY(overflowRequest.ParseFromArray(data.data(), data.size()));
    payments::PaymentDetails overflowDetails;
    QVERIFY(overflowDetails.ParseFromString(overflowRequest.serialized_payment_details()));
    QVERIFY(overflowDetails.outputs_size() > 0);
    for (int i = 0; i < overflowDetails.outputs_size(); ++i)
        overflowDetails.mutable_outputs(i)->set_amount(MAX_MONEY + 1);
    overflowRequest.set_serialized_payment_details(overflowDetails.SerializeAsString());
    const auto overflowData = overflowRequest.SerializeAsString();
    data.assign(overflowData.begin(), overflowData.end());
    byteArray = QByteArray((const char*)data.data(), data.size());
    r.paymentRequest.parse(byteArray);
    // Ensure the request is initialized
    QVERIFY(r.paymentRequest.IsInitialized());
    // Extract address and amount from the request
    QList<std::pair<CScript, CAmount> > sendingTos = r.paymentRequest.getPayTo();
    QVERIFY(!sendingTos.isEmpty());
    for (const std::pair<CScript, CAmount>& sendingTo : sendingTos) {
        CTxDestination dest;
        QVERIFY(ExtractDestination(sendingTo.first, dest));
        QCOMPARE(PaymentServer::verifyAmount(sendingTo.second), false);
    }
    QVERIFY(PaymentServer::verifyAmount(MAX_MONEY));
    QVERIFY(!PaymentServer::verifyAmount(-1));

    PaymentServer::LoadRootCAs(X509_STORE_new());
}

void RecipientCatcher::getRecipient(const SendCoinsRecipient& r)
{
    recipient = r;
}
