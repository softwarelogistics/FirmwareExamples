# WiFiHTTPS — NuvIoT HTTPS test bench

A small ESP32 PlatformIO project for exercising the NuvIoT library's trusted Wi-Fi HTTPS client.

## First startup

1. Build and upload the \`esp32dev\` environment.
2. Open the serial monitor at 115200 baud.
3. If SysConfig does not contain Wi-Fi credentials, the device prompts for the SSID and password.
4. The password is not echoed to the console.
5. Credentials are saved through the existing \`SysConfig\` SPIFFS persistence. Subsequent boots reuse them.
6. Once Wi-Fi connects, the example automatically requests \`https://example.com/\`.

The example uses the existing SysConfig storage format. It does not introduce a second EEPROM store. SysConfig currently persists its JSON configuration in SPIFFS; credentials are not encrypted at rest by this example.

## Console commands

| Command | Purpose |
|---|---|
| \`help\` | Show commands |
| \`https\` | Run the default HTTPS GET |
| \`https <url>\` | Test another HTTPS endpoint |
| \`https-expired\` | Exercise expired-certificate rejection |
| \`https-wrong-host\` | Exercise hostname validation |
| \`https-untrusted\` | Exercise untrusted-root rejection |
| \`wifi-status\` | Show Wi-Fi state and IP |
| \`wifi-reset\` | Clear saved credentials and prompt again |

Only \`https://\` URLs are accepted. Redirects are disabled and no HTTP downgrade is attempted.

## What this exercises

- ESP-IDF embedded full root CA bundle.
- TLS certificate-chain and hostname validation.
- Device clock synchronization and certificate validity dates.
- HTTPS GET using \`HTTPClient.begin(secureClient, url)\`.
- Explicit rejection of HTTP URLs, redirects and TLS failures.

The negative test URLs use the public BadSSL test service and therefore require internet access. They are optional; the default test uses \`example.com\`.

## Build

Open this directory in VS Code with PlatformIO, or run:

\`\`\`sh
pio run -e esp32dev
pio run -e esp32dev -t upload
pio device monitor -b 115200
\`\`\`

The PlatformIO environment pins the existing Arduino-ESP32 2.0.6 platform family used by the current DFU examples and consumes the NuvIoT HTTPS client from the matching feature branch while it is under review.
