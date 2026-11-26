#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <HTTPClient.h>

#include <Console.h>
#include <SysConfig.h>
#include <WiFiHttpsClient.h>

#define EXAMPLE_SKU "WIFI_HTTPS"
#define FIRMWARE_VERSION "1.0.0"
#define DEFAULT_HTTPS_URL "https://example.com/"

Console console(&Serial);
SysConfig sysConfig(&console);

enum class CredentialInput
{
    None,
    Ssid,
    Password
};

CredentialInput credentialInput = CredentialInput::None;
bool credentialsPendingSave = false;
bool wifiStarted = false;
bool automaticTestQueued = false;
bool httpsRequestPending = false;
String requestedHttpsUrl = DEFAULT_HTTPS_URL;
unsigned long nextWifiStatus = 0;

void promptForCredentials()
{
    credentialInput = sysConfig.WiFiSSID.length() == 0
        ? CredentialInput::Ssid
        : CredentialInput::Password;

    console.setEchoInput(credentialInput != CredentialInput::Password);

    if (credentialInput == CredentialInput::Ssid)
        console.println("First-run setup: enter Wi-Fi SSID:");
    else
        console.println("Enter Wi-Fi password (input is hidden):");
}

void startWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    console.println("Connecting to Wi-Fi SSID: " + sysConfig.WiFiSSID);
    WiFi.begin(sysConfig.WiFiSSID.c_str(), sysConfig.WiFiPWD.c_str());

    wifiStarted = true;
    nextWifiStatus = millis() + 1000;
}

void queueHttpsTest(String url)
{
    if (!url.startsWith("https://"))
    {
        console.printError("Only https:// URLs are accepted; HTTP downgrade is not supported.");
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        console.printError("Wi-Fi is not connected yet.");
        return;
    }

    requestedHttpsUrl = url;
    httpsRequestPending = true;
}

void handleConsoleCommand(String command)
{
    if (credentialInput == CredentialInput::Ssid)
    {
        if (command.length() == 0)
        {
            console.println("SSID cannot be empty. Enter Wi-Fi SSID:");
            return;
        }

        sysConfig.WiFiSSID = command;
        credentialInput = CredentialInput::Password;
        console.setEchoInput(false);
        console.println("Enter Wi-Fi password (input is hidden):");
        return;
    }

    if (credentialInput == CredentialInput::Password)
    {
        if (command.length() == 0)
        {
            console.println("Password cannot be empty. Enter Wi-Fi password:");
            return;
        }

        sysConfig.WiFiPWD = command;
        credentialInput = CredentialInput::None;
        console.setEchoInput(true);
        credentialsPendingSave = true;
        console.println("Credentials received. Saving to SysConfig...");
        return;
    }

    command.trim();

    if (command == "help")
    {
        console.println("Commands:");
        console.println("  https                  Run the default HTTPS GET test");
        console.println("  https <url>            Test another HTTPS endpoint");
        console.println("  https-expired          Test an expired certificate");
        console.println("  https-wrong-host       Test hostname validation");
        console.println("  https-untrusted        Test an untrusted certificate");
        console.println("  wifi-status            Show Wi-Fi connection status");
        console.println("  wifi-reset             Clear saved Wi-Fi credentials");
        return;
    }

    if (command == "https")
        queueHttpsTest(DEFAULT_HTTPS_URL);
    else if (command.startsWith("https "))
        queueHttpsTest(command.substring(6));
    else if (command == "https-expired")
        queueHttpsTest("https://expired.badssl.com/");
    else if (command == "https-wrong-host")
        queueHttpsTest("https://wrong.host.badssl.com/");
    else if (command == "https-untrusted")
        queueHttpsTest("https://untrusted-root.badssl.com/");
    else if (command == "wifi-status")
    {
        console.println("Wi-Fi status: " + String(static_cast<int>(WiFi.status())));
        if (WiFi.status() == WL_CONNECTED)
            console.println("IP address: " + WiFi.localIP().toString());
    }
    else if (command == "wifi-reset")
    {
        WiFi.disconnect();
        sysConfig.WiFiSSID = "";
        sysConfig.WiFiPWD = "";
        sysConfig.WiFiEnabled = false;
        sysConfig.Commissioned = false;
        sysConfig.write();
        wifiStarted = false;
        automaticTestQueued = false;
        console.println("Saved Wi-Fi credentials cleared.");
        promptForCredentials();
    }
    else if (command.length() > 0)
    {
        console.println("Unknown command. Enter help for available commands.");
    }
}

void saveCredentialsAndConnect()
{
    credentialsPendingSave = false;

    sysConfig.WiFiEnabled = true;
    sysConfig.Commissioned = true;
    sysConfig.write();

    console.println("Wi-Fi credentials saved using SysConfig (SPIFFS).");
    startWiFi();
}

void runHttpsTest(const String &url)
{
    httpsRequestPending = false;

    if (WiFi.status() != WL_CONNECTED)
    {
        console.printError("Wi-Fi disconnected before HTTPS test.");
        return;
    }

    console.println("[HTTPS] GET " + url);

    WiFiHttpsClient secureClient;
    if (!WiFiHttpsClient::synchronizeClock(NULL, &console))
    {
        console.printError("[HTTPS] Device clock is not valid. Request refused.");
        return;
    }

    HTTPClient request;
    request.setReuse(false);
    request.setConnectTimeout(10000);
    request.setTimeout(10000);
    request.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);

    if (!request.begin(secureClient, url))
    {
        console.printError("[HTTPS] Could not initialize HTTPS request.");
        request.end();
        return;
    }

    const int status = request.GET();
    if (status > 0)
    {
        console.println("[HTTPS] HTTP status: " + String(status));
        console.println("[HTTPS] Content length: " + String(request.getSize()));
        if (status >= 200 && status < 300)
            console.println("[HTTPS] Certificate and hostname validation succeeded.");
        else
            console.printWarning("[HTTPS] Endpoint returned a non-success status; redirects are disabled.");
    }
    else
    {
        console.printError("[HTTPS] Request failed: " + String(status));
        console.println("[HTTPS] No HTTP fallback was attempted.");
    }

    request.end();
}

void setup()
{
    Serial.begin(115200);
    console.enableSerialOut(true);
    console.enableBTOut(false);
    console.setEchoInput(true);
    console.registerCallback(handleConsoleCommand);

    console.println("");
    console.println("NuvIoT Wi-Fi HTTPS Test Bench");
    console.println("Firmware: " FIRMWARE_VERSION);

    if (!SPIFFS.begin(true))
    {
        console.printError("Could not initialize SPIFFS.");
        while (true)
            delay(1000);
    }

    sysConfig.load();

    if (sysConfig.WiFiSSID.length() == 0 || sysConfig.WiFiPWD.length() == 0)
    {
        promptForCredentials();
        return;
    }

    sysConfig.WiFiEnabled = true;
    sysConfig.Commissioned = true;
    startWiFi();

    console.println("Enter help for HTTPS and Wi-Fi test commands.");
}

void loop()
{
    console.loop();

    if (credentialsPendingSave)
        saveCredentialsAndConnect();

    if (wifiStarted && WiFi.status() == WL_CONNECTED && !automaticTestQueued)
    {
        automaticTestQueued = true;
        console.println("Wi-Fi connected. Starting initial HTTPS test.");
        queueHttpsTest(DEFAULT_HTTPS_URL);
    }

    if (wifiStarted && WiFi.status() != WL_CONNECTED && millis() >= nextWifiStatus)
    {
        console.println("Waiting for Wi-Fi. Status: " + String(static_cast<int>(WiFi.status())));
        nextWifiStatus = millis() + 5000;
    }

    if (httpsRequestPending)
        runHttpsTest(requestedHttpsUrl);
}
