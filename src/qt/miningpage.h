// Local opt-in CPU solo mining. No automatic start and no external miner process.
#ifndef MACHINECOIN_QT_MININGPAGE_H
#define MACHINECOIN_QT_MININGPAGE_H
#include <qt/clientmodel.h>
#include <qt/walletmodel.h>
#include <interfaces/node.h>
#include <univalue.h>
#include <QApplication>
#include <QElapsedTimer>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QSettings>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <atomic>
#include <mutex>
#include <vector>
#include <memory>
#include <set>

struct WalletMiningSession {
    std::mutex mutex;
    std::set<std::string> blockHashes;
};

class WalletMiningWorker : public QThread {
public:
    interfaces::Node& node;
    std::string address;
    std::shared_ptr<WalletMiningSession> session;
    std::atomic<bool> stop{false};
    std::atomic<unsigned> blocks{0}, batches{0};
    std::atomic<bool> waiting{false};
    std::mutex errorMutex;
    QString error;
    WalletMiningWorker(interfaces::Node& n, const std::string& a, std::shared_ptr<WalletMiningSession> s): node(n), address(a), session(std::move(s)) {}
    void run() override {
        while (!stop.load()) {
            try {
                // Apply the same peer, IBD and masternode-payment readiness checks
                // as external miners. Regtest is intentionally offline-capable.
                if (node.getNetwork() != "regtest") {
                    UniValue rules(UniValue::VARR); rules.push_back("segwit");
                    UniValue request(UniValue::VOBJ); request.pushKV("rules", rules);
                    UniValue params(UniValue::VARR); params.push_back(request);
                    node.executeRpc("getblocktemplate", params, "");
                }
                waiting = false;
                if (stop.load()) break;
                UniValue params(UniValue::VARR);
                params.push_back(1); params.push_back(address); params.push_back(256);
                auto result = node.executeRpc("generatetoaddress", params, "");
                {
                    std::lock_guard<std::mutex> lock(session->mutex);
                    for (const auto& hash : result.getValues()) {
                        if (session->blockHashes.insert(hash.get_str()).second) ++blocks;
                    }
                }
                ++batches;
                // Yield between batches, including on very low difficulty chains.
                for (unsigned pause = 0; pause < (result.empty() ? 1U : 50U) && !stop.load(); ++pause)
                    QThread::msleep(20);
            } catch (const UniValue& e) {
                int code = find_value(e, "code").get_int();
                if (code == -9 || code == -10 || code == -31) {
                    waiting = true;
                    QThread::msleep(250);
                    continue;
                }
                std::lock_guard<std::mutex> lock(errorMutex);
                error = QString::fromStdString(find_value(e, "message").get_str());
                break;
            } catch (const std::exception& e) {
                std::lock_guard<std::mutex> lock(errorMutex);
                error = QString::fromUtf8(e.what()); break;
            }
        }
    }
};

class MiningPage : public QWidget {
    ClientModel* client = nullptr;
    WalletModel* wallet = nullptr;
    QPushButton *start, *stop;
    QSpinBox* threads;
    QLineEdit* address;
    QLabel *status, *stats;
    QElapsedTimer elapsed;
    std::vector<std::unique_ptr<WalletMiningWorker>> workers;
    unsigned savedBlocks = 0, savedBatches = 0;
    void stopMining() {
        for (auto& w : workers) w->stop = true;
        for (auto& w : workers) { w->wait(); savedBlocks += w->blocks.load(); savedBatches += w->batches.load(); }
        workers.clear();
        start->setEnabled(client && wallet); stop->setEnabled(false); threads->setEnabled(true);
        status->setText(tr("Stopped. Mining will not restart automatically."));
        updateStats();
    }
    void updateStats() {
        unsigned blocks = savedBlocks, batches = savedBatches;
        for (auto& w : workers) { blocks += w->blocks.load(); batches += w->batches.load(); }
        stats->setText(tr("Accepted blocks this session: %1 · Completed work batches: %2").arg(blocks).arg(batches));
    }
    void refresh() {
        updateStats();
        if (workers.empty()) return;
        bool waiting = false;
        for (auto& w : workers) {
            QString error;
            { std::lock_guard<std::mutex> lock(w->errorMutex); error = w->error; }
            if (!error.isEmpty()) { stopMining(); status->setText(tr("Mining stopped: %1").arg(error)); return; }
            waiting |= w->waiting.load();
        }
        status->setText(waiting ? tr("Waiting for peers, blockchain or masternode payment synchronization…")
            : tr("CPU mining · %1 thread(s) · %2 seconds elapsed").arg(workers.size()).arg(elapsed.elapsed()/1000));
    }
    void startMining() {
        if (!client || !wallet || !workers.empty()) return;
        try {
            if (address->text().isEmpty()) {
                UniValue params(UniValue::VARR); params.push_back("CPU mining"); params.push_back("legacy");
                auto result = client->node().executeRpc("getnewaddress", params,
                    "/wallet/" + QUrl::toPercentEncoding(wallet->getWalletName()).toStdString());
                address->setText(QString::fromStdString(result.get_str()));
            }
            savedBlocks = savedBatches = 0; elapsed.start();
            auto session = std::make_shared<WalletMiningSession>();
            for (int i=0; i<threads->value(); ++i) {
                workers.emplace_back(new WalletMiningWorker(client->node(), address->text().toStdString(), session));
                workers.back()->start(QThread::LowPriority);
            }
            start->setEnabled(false); stop->setEnabled(true); threads->setEnabled(false);
            status->setText(tr("Starting CPU miner…"));
        } catch (const UniValue& e) {
            stopMining(); status->setText(QString::fromStdString(find_value(e,"message").get_str()));
        } catch (const std::exception& e) {
            stopMining(); status->setText(QString::fromUtf8(e.what()));
        }
    }
public:
    explicit MiningPage(QWidget* parent=nullptr): QWidget(parent) {
        auto layout = new QVBoxLayout(this); layout->setSpacing(16);
        auto description = new QLabel(tr("Solo mine Machinecoin with your CPU. Rewards go to this wallet only when a block is accepted. Mining uses power and produces heat."));
        description->setWordWrap(true); layout->addWidget(description);
        layout->addWidget(new QLabel(tr("CPU threads")));
        threads = new QSpinBox; threads->setRange(1, qMax(1, QThread::idealThreadCount())); threads->setValue(qBound(1, QSettings().value("mining/threads", 1).toInt(), threads->maximum()));
        connect(threads, QOverload<int>::of(&QSpinBox::valueChanged), this, [](int value) { QSettings().setValue("mining/threads", value); }); threads->setMaximumWidth(180);
        layout->addWidget(threads);
        layout->addWidget(new QLabel(tr("Reward address (created in this wallet when you start)")));
        address = new QLineEdit; address->setReadOnly(true); layout->addWidget(address);
        auto buttons = new QHBoxLayout;
        start = new QPushButton(tr("Start CPU mining")); start->setObjectName("startCpuMining");
        stop = new QPushButton(tr("Stop mining")); stop->setObjectName("stopCpuMining"); stop->setEnabled(false);
        auto gpu = new QPushButton(tr("GPU mining — unavailable")); gpu->setEnabled(false);
        buttons->addWidget(start); buttons->addWidget(stop); buttons->addWidget(gpu); layout->addLayout(buttons);
        auto gpuInfo = new QLabel(tr("No TimeTravel GPU backend is included in this build. CPU mining is available; GPU mining requires a separate compatible backend."));
        gpuInfo->setWordWrap(true); layout->addWidget(gpuInfo);
        status = new QLabel(tr("Stopped. Mining starts only when you press Start CPU mining.")); status->setWordWrap(true); layout->addWidget(status);
        stats = new QLabel; stats->setWordWrap(true); layout->addWidget(stats); updateStats();
        layout->addStretch();
        connect(start, &QPushButton::clicked, this, [this] { startMining(); });
        connect(stop, &QPushButton::clicked, this, [this] { stopMining(); });
        connect(qApp, &QCoreApplication::aboutToQuit, this, [this] { stopMining(); });
        auto timer = new QTimer(this); connect(timer, &QTimer::timeout, this, [this] { refresh(); }); timer->start(500);
    }
    ~MiningPage() override { stopMining(); }
    void setClientModel(ClientModel* c) { if (!c) stopMining(); client=c; start->setEnabled(client && wallet); }
    void setWalletModel(WalletModel* w) { stopMining(); wallet=w; address->clear(); start->setEnabled(client && wallet); }
};
#endif
