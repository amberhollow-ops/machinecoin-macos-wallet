// Local presentation-only styling. No wallet or chain behavior changes.
#ifndef MACHINECOIN_QT_MODERNSTYLE_H
#define MACHINECOIN_QT_MODERNSTYLE_H
#include <QApplication>
#include <QPalette>
inline void applyModernWalletStyle()
{
    QApplication::setStyle(QStringLiteral("Fusion"));
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Window, QColor("#111314"));
    palette.setColor(QPalette::WindowText, QColor("#f4f4f1"));
    palette.setColor(QPalette::Base, QColor("#1d2022"));
    palette.setColor(QPalette::AlternateBase, QColor("#25292c"));
    palette.setColor(QPalette::Text, QColor("#f4f4f1"));
    palette.setColor(QPalette::Button, QColor("#25292c"));
    palette.setColor(QPalette::ButtonText, QColor("#f4f4f1"));
    palette.setColor(QPalette::Link, QColor("#ffc24b"));
    palette.setColor(QPalette::LinkVisited, QColor("#ffc24b"));
    palette.setColor(QPalette::Highlight, QColor("#ffc24b"));
    palette.setColor(QPalette::HighlightedText, QColor("#111314"));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#777f87"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#777f87"));
    QApplication::setPalette(palette);
    qApp->setStyleSheet(QStringLiteral(R"QSS(
        QWidget { font-size: 13px; }
        QMainWindow { background: #111314; }
        QLabel { color: #f4f4f1; }
        QLabel#pageTitle { font-size: 28px; font-weight: 600; }
        QLabel#pageSubtitle { color: #a6adb5; font-size: 13px; }
        QToolBar#navigation { background: #16191b; border: none; padding: 18px 10px; spacing: 8px; }
        QToolBar#navigation QToolButton { color: #cbd0d5; background: transparent; border: none; border-radius: 7px; padding: 13px 14px; text-align: left; min-width: 136px; font-size: 14px; }
        QToolBar#navigation QToolButton:hover { background: #292b2c; }
        QToolBar#navigation QToolButton:checked { background: #3b3220; color: white; font-weight: 600; }
        QToolBar#navigation QToolButton:focus { border: 1px solid #ffc24b; }
        QLabel#brand { color: white; font-size: 20px; font-weight: 600; padding: 8px 12px 0px; }
        QLabel#brandCaption { color: #a6adb5; font-size: 11px; padding: 0px 12px 24px; }
        QToolBar#navigation QLabel { color: #cbd0d5; }
        QFrame#balanceCard, QFrame#activityCard, ReceiveCoinsDialog QFrame#frame, ReceiveCoinsDialog QFrame#frame2, QFrame#frameFee, QFrame#frameCoinControl { background: #1d2022; border: 1px solid #33383d; border-radius: 20px; }
        QLabel#label_5, QLabel#label_4 { font-size: 16px; font-weight: 600; }
        QLabel#labelTotal { font-size: 32px; color: #f4f4f1; font-weight: 600; }
        QLabel#emptyActivity { color: #a6adb5; padding: 24px 8px; }
        QLineEdit, QComboBox, QTextEdit, QPlainTextEdit, QAbstractSpinBox { background: #1d2022; color: #f4f4f1; border: 1px solid #3d444a; border-radius: 5px; padding: 6px; min-height: 20px; selection-background-color: #ffc24b; }
        QAbstractSpinBox { min-height: 30px; }
        QLineEdit:focus, QComboBox:focus { border: 1px solid #ffc24b; }
        QLineEdit:disabled { background: #25292c; color: #777f87; }
        QPushButton { background: #1d2022; color: #f4f4f1; border: 1px solid #3d444a; border-radius: 5px; padding: 7px 14px; min-height: 20px; }
        QPushButton:hover { background: #303437; border-color: #ffc24b; }
        QPushButton:focus { border: 2px solid #ffc24b; padding: 6px 13px; }
        QPushButton:disabled { background: #25292c; color: #777f87; border-color: #33383d; }
        QPushButton#sendButton, QPushButton#receiveButton { background: #ffc24b; color: #111314; border-color: #ffc24b; }
        QPushButton#sendButton:disabled, QPushButton#receiveButton:disabled { background: #33383d; color: #777f87; border-color: #33383d; }
        QPushButton#labelWalletStatus, QPushButton#labelTransactionsStatus { padding: 0; border: none; background: transparent; min-height: 24px; }
        QTableView, QListView { background: #1d2022; alternate-background-color: #25292c; border: 1px solid #33383d; border-radius: 6px; selection-background-color: #ffc24b; selection-color: #111314; }
        QHeaderView::section { background: #25292c; color: #a6adb5; border: none; border-bottom: 1px solid #33383d; padding: 9px 7px; font-weight: 600; }
        QTableView::item { padding: 5px; }
        QStatusBar { background: #191c1e; border-top: 1px solid #33383d; padding: 6px 12px; }
        QStatusBar::item { border: none; }
        QStatusBar QLabel { font-size: 12px; }
        QLabel#peerStatus { font-weight: 600; padding: 0 8px; }
        QLabel#chainStatus { color: #a6adb5; padding: 0 8px; }
        QProgressBar { border: 1px solid #3d444a; border-radius: 4px; background: #1d2022; text-align: center; min-height: 18px; }
        QProgressBar::chunk { background: #b28a37; }
        QTabWidget::pane { border: 1px solid #33383d; background: #1d2022; border-radius: 7px; }
        QTabBar::tab { padding: 10px 16px; background: #191c1e; color: #a6adb5; }
        QTabBar::tab:selected { background: #ffc24b; color: #111314; }
        QLabel#masternodeSyncStatus { background: #25292c; border-radius: 6px; padding: 10px; color: #a6adb5; }
        QScrollBar:horizontal { background: #191c1e; height: 12px; margin: 0; border-radius: 5px; }
        QScrollBar::handle:horizontal { background: #56616b; min-width: 32px; border-radius: 5px; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }
        QToolTip { background: #303437; color: #f4f4f1; border: none; padding: 7px; }
        QDialog, QMessageBox { background: #111314; }
        QMenu { background: #1d2022; color: #f4f4f1; border: 1px solid #3d444a; padding: 6px; }
        QMenu::item:selected { background: #3b3220; color: #ffc24b; }
        QGroupBox { border: 1px solid #33383d; border-radius: 10px; margin-top: 12px; padding-top: 12px; }
        QGroupBox::title { color: #a6adb5; subcontrol-origin: margin; left: 12px; }
        QScrollBar:vertical { background: #191c1e; width: 12px; }
        QScrollBar::handle:vertical { background: #56616b; min-height: 28px; border-radius: 5px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QLabel#lblQRCode { background: white; border-radius: 0; }
    )QSS"));
}
#endif
