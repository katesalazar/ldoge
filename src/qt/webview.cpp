/*
 * Qt5 webview naviagation
 *
 * Developed by OnsightIT 2014-2015
 * onsightit@gmail.com
 */
#include "<qt/webview.h"
#include "<qt/util.h"

#include <qt/overviewpage.h>
#include <qt/platformstyle.h>
#include <qt/receivecoinsdialog.h>
#include <qt/sendcoinsdialog.h>
#include <qt/signverifymessagedialog.h>
#include <qt/transactiontablemodel.h>
#include <qt/transactionview.h>
#include <qt/walletmodel.h>
#include <qt/addressbookpage.h>
#include <qt/askpassphrasedialog.h>
#include <qt/bitcoingui.h>
#include <qt/clientmodel.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>

#include <ui_interface.h>

#include <QWebView>
#include <QWebFrame>
#include <QWebHistory>
#include <QDesktopServices>
#include <QPushButton>
#include <QList>
#include <QUrl>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>


using namespace std;

WebView::WebView(QWidget *parent) :
    QWebView(parent)
{
    trustedUrls << "www.litedogeofficial.org" << "litedogeofficial.org";  // These will get appended to by the values in VERSION.json
    fTrustedUrlsSet = true;
}

WebView::~WebView()
{
}

// Receives web nav buttons from parent webview
void WebView::sendButtons(QPushButton *bb, QPushButton *hb, QPushButton *fb)
{
    // Get the addresses of the nav buttons
    backButton = bb;
    homeButton = hb;
    forwardButton = fb;
}

void WebView::myBack()
{
    if (this->history()->currentItemIndex() > 1) // 0 is a blank page
    {
        this->back();
    }
    setButtonStates((this->history()->currentItemIndex() > 1), (this->history()->currentItemIndex() > 1), this->history()->canGoForward());
}

void WebView::myHome()
{
    if ((this->history()->currentItemIndex() > 1))
    {
        this->history()->goToItem(this->history()->itemAt(1)); // 0 is a blank page
    }
    setButtonStates((this->history()->currentItemIndex() > 1), (this->history()->currentItemIndex() > 1), this->history()->canGoForward());
}

void WebView::myForward()
{
    if (this->history()->canGoForward())
    {
        this->forward();
    }
    setButtonStates((this->history()->currentItemIndex() > 1), (this->history()->currentItemIndex() > 1), this->history()->canGoForward());
}

void WebView::myReload()
{
    this->reload();
}

void WebView::myOpenUrl(QUrl url)
{
    if (!fTrustedUrlsSet)
    {
        std::string urls = GetArg("-vTrustedUrls", "");
        typedef vector<string> parts_type;
        parts_type parts;
        boost::split(parts, urls, boost::is_any_of(",; "), boost::token_compress_on);
        for (vector<string>::iterator it = parts.begin(); it != parts.end(); ++it)
        {
            QString url = QString::fromStdString(*it);
            // Sanity check the url
            if (url.contains(QChar('.')))
            {
                trustedUrls << url;
                if (!fTrustedUrlsSet)
                    fTrustedUrlsSet = true;
            }
        }
    }

    if (isTrustedUrl(url))
    {
        try
        {
            this->load(url);
        }
        catch (...)
        {
            printf("WebView: Error loading: %s\n", url.toString().toStdString().c_str());
        }
        // This uses canGoBack() and currentItemIndex > 0 as opposed to currentItemIndex > 1 like the other setButtonStates calls.
        setButtonStates(this->history()->canGoBack(), (this->history()->currentItemIndex() > 0), this->history()->canGoForward());
    }
    else
    {
        QDesktopServices::openUrl(url);
    }
}

// Set button enabled/disabled states
void WebView::setButtonStates(bool canGoBack, bool canGoHome, bool canGoForward)
{
    backButton->setEnabled(canGoBack);
    homeButton->setEnabled(canGoHome);
    forwardButton->setEnabled(canGoForward);
}

bool WebView::isTrustedUrl(QUrl url)
{
    if (trustedUrls.contains(url.host()))
        return true;
    else
        return false;
}

void WebView::sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist)
{
    qnr->ignoreSslErrors();
}
