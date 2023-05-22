// Sensor real-time data
let sensorData = [];
let svg = null;
let xScale = null;
let yScaleLinear = null;
let yScaleAngular = null;
let colorScale = null;
let gravity = { x: 0, y: 0, z: 0 };
let isAtRest = false;
let highScore = 1;

function log(text) {
    console.log(text);
    let row = document.getElementById('log');
    row.innerHTML += text + '\n';
}

function startMonitor() {

    // List all devices found
    let filters = [];
    filters.push({ name: "SENSOR_PRO" });

    let options = {};
    // options.acceptAllDevices = true;
    options.filters = filters;
    options.optionalServices = ['6a806050-b5a3-f393-e0a9-e50e24dcca9e', '6a800001-b5a3-f393-e0a9-e50e24dcca9e'];

    log('Requesting Bluetooth Device...');
    log('with ' + JSON.stringify(options));
    navigator.bluetooth.requestDevice(options)
        .then(device => {
            log('> Name:             ' + device.name);
            log('> Id:               ' + device.id);
            log('> Connected:        ' + device.gatt.connected);
            return device.gatt.connect();
        })
        .then(server => {
            log('> Connected to device:        ' + server.device.gatt.connected);
            log('> Getting MPU-5060 Service:   ' + server.connected);
            // Getting MPU-5060 Service…
            return server.getPrimaryService('6a800001-b5a3-f393-e0a9-e50e24dcca9e');
        })
        .then(service => {
            log('> Connected to service:        ' + service);
            // Getting MPU-5060 Characteristic…
            return service.getCharacteristic('6a806050-b5a3-f393-e0a9-e50e24dcca9e');
        })
        .then(characteristic => {
            // Handle Notifications MPU-5060…
            log('> Start notifications MPU-5060:            ' + characteristic);
            return characteristic.startNotifications();
        })
        .then(characteristic => {
            log('> Notifications started MPU-5060:          ' + characteristic);
            characteristic.addEventListener('characteristicvaluechanged', handleChangedValue);
        })
        .catch(error => {
            log('Error: ' + error);
        });

    function updateGravity(x, y, z) {
        if (isDeviceAtRest(x, y, z)) {
            console.log('Device is at rest: ' + x + ', ' + y + ', ' + z);
            gravity.x = x;
            gravity.y = y;
            gravity.z = z;
        }
    }

    function removeGravity(x, y, z) {
        return {
            x: x - gravity.x,
            y: y - gravity.y,
            z: z - gravity.z
        };
    }

    function isDeviceAtRest(x, y, z) {
        // For simplicity, here we return true if the magnitude is close to 1g.
        let magnitude = Math.sqrt(x * x + y * y + z * z);
        return Math.abs(magnitude - 1) < 0.35;
    }

    function handleChangedValue(event) {
        let value = event.target.value;
        let x = value.getInt16(0, false) / 16384;
        let y = value.getInt16(2, false) / 16384;
        let z = value.getInt16(4, false) / 16384;
        let X = value.getInt16(6, false) / 131;
        let Y = value.getInt16(8, false) / 131;
        let Z = value.getInt16(10, false) / 131;

        updateGravity(x, y, z);

        let acceleration = removeGravity(x, y, z);

        let magnitude = Math.sqrt(acceleration.x * acceleration.x + acceleration.y * acceleration.y + acceleration.z * acceleration.z);

        // Add new data to sensorData
        let date = new Date();
        sensorData.push({
            date: date,
            x: acceleration.x,
            y: acceleration.y,
            z: acceleration.z,
            m: magnitude,
        });

        let tbody = document.querySelector('#sensorDataBody');
        let row = document.createElement('tr');
        row.innerHTML = `<td>${date}</td><td>${x.toFixed(2)}</td><td>${y.toFixed(2)}</td><td>${z.toFixed(2)}</td><td>${X.toFixed(2)}</td><td>${Y.toFixed(2)}</td><td>${Z.toFixed(2)}</td>`;
        tbody.appendChild(row);

        // Call updateChart to reflect new data
        updateChart();
        updateHighScore(magnitude);
    }

};

function random(min, max) {
    return Math.random() * (max - min) + min;
}

function resetChartData() {
    sensorData = [];
    highScore = 0;
    const numberElement = document.getElementById('highScore');
    numberElement.innerText = highScore;
    setAtRest();
}

function setAtRest() {
    atRest = true;
}

function simData() {

    let date = new Date();
    sensorData.push({
        date: date,
        x: random(-10, 10),
        y: random(-10, 10),
        z: random(-10, 10)
    });
    updateChart();

}