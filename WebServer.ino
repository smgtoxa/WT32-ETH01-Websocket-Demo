#include <WebServer_WT32_ETH01.h>
#include <WebSocketsServer.h>

#define DEBUG_ETHERNET_WEBSERVER_PORT Serial

// Debug Level from 0 to 4
#define _ETHERNET_WEBSERVER_LOGLEVEL_ 3

WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);

// HTML content to be served
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>WebSocket Test</title>
</head>
<body>
  <h2>WebSocket Test</h2>
  <div>
    <input type="text" id="messageInput" placeholder="Enter message">
    <button onclick="sendMessage()">Send</button>
  </div>
  <div>
    <h3>Messages</h3>
    <pre id="messages"></pre>
  </div>
  <script>
    var websocket;
    window.onload = function() {
      websocket = new WebSocket('ws://' + window.location.hostname + ':81');
      websocket.onopen = function() {
        document.getElementById('messages').innerText += 'Connected to WebSocket server\n';
      };
      websocket.onmessage = function(event) {
        document.getElementById('messages').innerText += 'Received: ' + event.data + '\n';
      };
      websocket.onclose = function() {
        document.getElementById('messages').innerText += 'Disconnected from WebSocket server\n';
      };
    };

    function sendMessage() {
      var message = document.getElementById('messageInput').value;
      websocket.send(message);
      document.getElementById('messages').innerText += 'Sent: ' + message + '\n';
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_TEXT) {
    Serial.write(payload, length); // Send to RS232
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.print("\nStarting WebServer on WT32-ETH01");
  Serial.println(WEBSERVER_WT32_ETH01_VERSION);

  // To be called before ETH.begin()
  WT32_ETH01_onEvent();

  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);

  WT32_ETH01_waitForConnect();

  // Print the obtained IP address to RS232
  Serial.print("IP Address: ");
  Serial.println(ETH.localIP());

  // Start the web server
  server.on("/", handleRoot);
  server.begin();

  // Start the WebSocket server
  webSocket.begin();
  webSocket.onEvent(handleWebSocketMessage);
}

void loop() {
  // Handle WebSocket clients
  webSocket.loop();

  // Handle HTTP clients
  server.handleClient();

  // Check for RS232 data and send it over WebSocket
  if (Serial.available() > 0) {
    String serialData = Serial.readStringUntil('\n'); // Read the incoming data
    webSocket.broadcastTXT(serialData); // Broadcast the data to all WebSocket clients
  }
}
