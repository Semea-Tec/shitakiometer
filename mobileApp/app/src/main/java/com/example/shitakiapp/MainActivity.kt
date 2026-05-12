package com.example.shitakiapp

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.shitakiapp.ui.theme.ShitakiappTheme
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.net.URL

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            ShitakiappTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    EspMonitorScreen(modifier = Modifier.padding(innerPadding))
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun EspMonitorScreen(modifier: Modifier = Modifier) {
    var ipAddress by remember { mutableStateOf("192.168.0.") }
    var isRunning by remember { mutableStateOf(false) }
    var temperature by remember { mutableStateOf("--") }
    var humidity by remember { mutableStateOf("--") }
    var co2 by remember { mutableStateOf("--") }
    var rssi by remember { mutableStateOf("--") }
    var errorMessage by remember { mutableStateOf("") }
    
    val coroutineScope = rememberCoroutineScope()

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text(
            text = "Shitakiometer",
            style = MaterialTheme.typography.headlineMedium,
            color = MaterialTheme.colorScheme.primary,
            fontWeight = FontWeight.Bold,
            modifier = Modifier.padding(bottom = 24.dp, top = 16.dp)
        )

        Card(
            modifier = Modifier.fillMaxWidth(),
            elevation = CardDefaults.cardElevation(defaultElevation = 6.dp),
            shape = RoundedCornerShape(16.dp)
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                OutlinedTextField(
                    value = ipAddress,
                    onValueChange = { ipAddress = it },
                    label = { Text("ESP IP Address") },
                    modifier = Modifier.fillMaxWidth(),
                    singleLine = true,
                    shape = RoundedCornerShape(12.dp)
                )
                
                Spacer(modifier = Modifier.height(16.dp))
                
                Button(
                    onClick = {
                        isRunning = !isRunning
                        if (isRunning) {
                            errorMessage = ""
                            coroutineScope.launch {
                                while (isActive && isRunning) {
                                    try {
                                        val result = withContext(Dispatchers.IO) {
                                            URL("http://$ipAddress/data").readText()
                                        }
                                        val json = JSONObject(result)
                                        temperature = json.optString("temperature", "--")
                                        humidity = json.optString("humidity", "--")
                                        co2 = json.optString("co2", "--")
                                        rssi = json.optString("rssi", "--")
                                        errorMessage = ""
                                    } catch (e: Exception) {
                                        errorMessage = e.localizedMessage ?: "Unknown Error"
                                    }
                                    delay(2000)
                                }
                            }
                        }
                    },
                    modifier = Modifier.fillMaxWidth().height(50.dp),
                    shape = RoundedCornerShape(12.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = if (isRunning) MaterialTheme.colorScheme.error else MaterialTheme.colorScheme.primary
                    )
                ) {
                    Text(
                        if (isRunning) "Stop Monitoring" else "Connect & Monitor", 
                        style = MaterialTheme.typography.titleMedium
                    )
                }
            }
        }
        
        Spacer(modifier = Modifier.height(24.dp))
        
        if (errorMessage.isNotEmpty()) {
            Card(
                colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.errorContainer),
                modifier = Modifier.fillMaxWidth().padding(bottom = 16.dp),
                shape = RoundedCornerShape(12.dp)
            ) {
                Text(
                    text = "Error: $errorMessage", 
                    color = MaterialTheme.colorScheme.onErrorContainer,
                    modifier = Modifier.padding(16.dp)
                )
            }
        }

        // 2x2 Grid for Sensor Data
        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(16.dp)) {
            SensorCard(
                title = "Temperature",
                value = temperature,
                unit = "°C",
                emoji = "🌡️",
                modifier = Modifier.weight(1f)
            )
            SensorCard(
                title = "Humidity",
                value = humidity,
                unit = "%",
                emoji = "💧",
                modifier = Modifier.weight(1f)
            )
        }
        Spacer(modifier = Modifier.height(16.dp))
        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(16.dp)) {
            SensorCard(
                title = "CO2 Level",
                value = co2,
                unit = "ppm",
                emoji = "☁️",
                modifier = Modifier.weight(1f)
            )
            SensorCard(
                title = "Signal (RSSI)",
                value = rssi,
                unit = "dBm",
                emoji = "📶",
                modifier = Modifier.weight(1f)
            )
        }
    }
}

@Composable
fun SensorCard(title: String, value: String, unit: String, emoji: String, modifier: Modifier = Modifier) {
    Card(
        modifier = modifier,
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceVariant),
        shape = RoundedCornerShape(16.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Text(text = emoji, fontSize = 32.sp)
            Spacer(modifier = Modifier.height(8.dp))
            Text(text = title, style = MaterialTheme.typography.labelMedium)
            Spacer(modifier = Modifier.height(4.dp))
            Text(
                text = if (value == "--") "--" else "$value $unit", 
                style = MaterialTheme.typography.titleLarge,
                color = MaterialTheme.colorScheme.primary,
                fontWeight = FontWeight.Bold
            )
        }
    }
}