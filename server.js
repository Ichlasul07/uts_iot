const express = require('express');
const mqtt = require('mqtt');
const cors = require('cors');  // Menggunakan CORS untuk izin akses
const app = express();
const port = 4000;

// Setup CORS untuk memperbolehkan permintaan dari frontend
app.use(cors());

// MQTT Client setup
const mqttClient = mqtt.connect('mqtt://broker.emqx.io', {
  username: 'ichlasul',
  password: 'ichlasul07',
});

const topic = 'UTS_IOT/emqx/esp32';

// Menyimpan data sensor yang diterima
let sensorData = {
  temperature: null,
  humidity: null,
};

// MQTT client connection handler
mqttClient.on('connect', () => {
  console.log('Connected to MQTT broker');
  mqttClient.subscribe(topic, (err) => {
    if (err) {
      console.error('Failed to subscribe to topic', err);
    }
  });
});

// MQTT message handler
mqttClient.on('message', (topic, message) => {
  try {
    const payload = JSON.parse(message.toString());
    sensorData = payload;
    console.log('Sensor Data Updated:', sensorData);
  } catch (err) {
    console.error('Error parsing message:', err);
  }
});

// API endpoint untuk mendapatkan data sensor terbaru
app.get('/api/sensorData', (req, res) => {
  if (sensorData.temperature === null || sensorData.humidity === null) {
    res.status(500).json({ error: 'Data not available' }); // Jika data belum tersedia
  } else {
    res.json(sensorData);
  }
});

// Menyajikan file statis jika dibutuhkan (misalnya file HTML di folder public)
app.use(express.static('public'));

// Mengaktifkan server Express
app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});
