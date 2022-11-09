# IoT Face recognizer based on ESP32-CAM board
Project uses ESP32-CAM to detect and recognize faces, and publish results on MQTT server.
With this project, user is able to enroll and delete faces that are stored in flash, using appropriate MQTT commands. Also, network and MQTT credentials are stored on the device.

# Setup
Prerequisites:
- Arduino IDE,
- ESP32-CAM board.

1. In Arduino IDE go to: Tools -> Board -> Board manager, and install **esp32** package in **version 1.0.4**
   **IMPORTANT:** Other versions will not work with the program

2. Copy the `FaceReg.csv` file to: `C:\Users\<username>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\tools\partitions\`

3. Open project in the Arduino IDE

4. In Tools -> Board, select `ESP32 Wrover module`
   Other options I use:
   - `Upload speed: 921600`
   - `Flash frequency: 80MHz`
   - `Flash mode: QIO`
   Don't forget to assign proper COM port


5. As a partition scheme, select `Face Recognition`

6. Upload the code to the device

At first launch, device will setup WiFi access point, to which user should connect (password: `admin`).
Next, using web browser, user should go to IP address that has been printed on UART (most likely 192.168.4.1).
Then, user should fill the form with WiFi and MQTT credentials. The device will check if WiFi credentials were put in correctly.

After checking WiFi credentials, the device will store them in EEPROM memory and it reset itself.
Device should connect to specified WiFi and MQTT server (process can be observed on UART).
If it fails, device will setup access point again.

If required, some properties can be changed in `config.h` file.

# Usage
After connecting to MQTT server, device will subscribe to two topics: `recognition_control` and `recognition_result`.
NOTE: Those are default topic names, they can be changed in `config.h`

In `recognition_control` the facial detection and recognition process can be controlled. User can start/stop the process by posting `run`/`stop` in the topic.
When the process is running, the device posts the results in `recognition_result` topic. To start enrolling detected face, `enroll` message has to be sent. To delete recognized face, a `delete` message should be posted.
However, face deletion has a limitation (caused by implementation of Espressif face recognition library), that user should delete faces in reverse order of enrolling (delete newest enrolled face first). In other case, during the next enroll some other enrolled face will be overriten.