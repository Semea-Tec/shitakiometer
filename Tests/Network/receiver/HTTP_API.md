# LoRa Receiver HTTP API Documentation

## Overview
The updated receiver code now includes an HTTP web server that allows mobile apps and other clients to retrieve sensor data via HTTP requests while maintaining LoRa reception and MQTT publishing.

## Features Added
- **HTTP Web Server** on port 80
- **Two API endpoints** for data retrieval
- **Data storage** system that saves the latest received sensor reading
- **Automatic timestamp** for each reading

## Available Endpoints

### 1. Home Page
**URL:** `http://<ESP32_IP_ADDRESS>/`

**Method:** GET

**Response:** HTML page with:
- Current WiFi IP address
- MQTT connection status
- Latest received sensor data
- Latest RSSI value
- Links to API endpoints

**Example:**
```
curl http://192.168.15.X/
```

### 2. Get Latest Data (JSON)
**URL:** `http://<ESP32_IP_ADDRESS>/data`

**Method:** GET

**Response Format:** JSON
```json
{
  "value": "temperature:25.5",
  "rssi": -95,
  "timestamp": 1234567890
}
```

**Response Headers:** `Content-Type: application/json`

**Example:**
```bash
curl http://192.168.15.X/data
```

**Mobile App Example (JavaScript/Fetch API):**
```javascript
fetch('http://<ESP32_IP_ADDRESS>/data')
  .then(response => response.json())
  .then(data => {
    console.log('Value:', data.value);
    console.log('RSSI:', data.rssi, 'dBm');
    console.log('Timestamp:', new Date(data.timestamp));
  });
```

**Mobile App Example (Flutter/Dart):**
```dart
import 'package:http/http.dart' as http;
import 'dart:convert';

Future<void> fetchSensorData() async {
  try {
    final response = await http.get(
      Uri.parse('http://<ESP32_IP_ADDRESS>/data'),
    );
    if (response.statusCode == 200) {
      final data = jsonDecode(response.body);
      print('Value: ${data["value"]}');
      print('RSSI: ${data["rssi"]} dBm');
      print('Timestamp: ${data["timestamp"]}');
    }
  } catch (e) {
    print('Error: $e');
  }
}
```

## Data Structure
The receiver stores sensor data in the following format:

```cpp
struct SensorData {
    String value;      // The actual sensor data (e.g., "CO2:412ppm")
    int rssi;          // Signal strength in dBm
    long timestamp;    // Milliseconds since ESP32 startup
}
```

## How It Works

1. **LoRa Reception** - ESP32 receives data from LoRa sender
2. **Data Storage** - Received data is stored in `latestData` struct with RSSI and timestamp
3. **MQTT Publishing** - Data is published to MQTT broker (existing functionality)
4. **OLED Display** - Data is shown on the display (existing functionality)
5. **HTTP Requests** - Mobile apps can request the data via HTTP endpoints

## Integration with Existing Features

The HTTP API runs **alongside** the existing functionality:
- ✅ LoRa reception continues to work
- ✅ MQTT publishing continues to work
- ✅ OLED display continues to update
- ✅ All at the same time!

## Serial Output
You'll see additional debug messages:
```
[HTTP] Web server started on http://192.168.15.X:80
[HTTP] GET / - Home page sent to client
[HTTP] GET /data - Data sent to client
```

## Network Requirements

1. **WiFi Connection:** ESP32 must be connected to WiFi (already required for MQTT)
2. **Same Network:** Mobile app must be on the same network or have route to ESP32
3. **Open Port 80:** Port 80 must be accessible (default HTTP port)

## Troubleshooting

### Can't connect to HTTP endpoints?
- Check if ESP32 is connected to WiFi
- Verify the correct IP address (check Serial monitor)
- Ensure mobile device is on the same network
- Check firewall settings

### Getting empty data?
- Wait for at least one LoRa packet to be received
- Check Serial monitor for `[LoRa] Data received!` message

### 404 errors on wrong endpoints?
- Use only `/` or `/data` endpoints
- Other paths will return 404

## Performance Considerations

- HTTP requests are non-blocking
- No impact on LoRa reception or MQTT publishing
- Each data point includes timestamp for tracking
- RSSI value is captured at the moment of reception

## Future Enhancements (Optional)

You could extend this API with:
- `/history` - Return last N readings
- `/config` - GET/POST configuration settings
- `/status` - Return detailed system status
- `/restart` - Restart the device
- CORS headers for cross-origin requests
- Authentication/API keys for security
