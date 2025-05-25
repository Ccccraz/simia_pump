## Simia pump

### Introduction
simia pump is a pump based on HID protocol, supports hot plug, compared with serial port, it does not need additional drivers, does not need to identify the port number, and can support multiple devices to use at the same time.

### Interface
for matlab: [PTBSimia](https://github.com/Ccccraz/PTBSimia)
for unity: [PsychoUnity](https://github.com/Ccccraz/PsychoUnity)

### Basic conccepts
#### Reward task queue
simia pump maintains a task list, adds new reward tasks to the device through hid, and realizes non-blocking operation in matlab.

```plain
╔══════════════════════════════════════════════════════╗
║                  REWARD TASK QUEUE                   ║
╠══════════╦══════════╦══════════╦══════════╦══════════╣
║ >task 001║  task 002║  task 003║  task ...║  task 100║
║  1000  ms║  100   ms║  1200  ms║  ...   ms║  3000  ms║
╠══════════╩══════════╩══════════╩══════════╩══════════╣
║    >>>>>>>>> SEQUENTIAL EXECUTION FLOW >>>>>>>>>     ║
╚══════════════════════════════════════════════════════╝
```

##### Add a reward task

```plain
╔══════════════════════════════════════════════════════╗
║                  REWARD TASK QUEUE                   ║
╠══════════╦══════════╦══════════╦══════════╦══════════╣
║ task 001 ║ new task ║      ... ║      ... ║      ... ║
║ 1000  ms ║ 100   ms ║          ║          ║          ║
╠══════════╬══════════╬══════════╬══════════╬══════════╣
║          ║     ↑    ║          ║          ║          ║
║          ║ add task ║          ║          ║          ║
╚══════════╩══════════╩══════════╩══════════╩══════════╝
```

##### Stop a reward task

```plain
╔══════════════════════════════════════════════════════╗
║         REWARD TASK QUEUE (STOPPING CURRENT)         ║
╠══════════╦══════════╦══════════╦══════════╦══════════╣
║ >task 001║  task 002║  task 003║  task ...║  task 100║
║  cancel  ║  100   ms║  1200  ms║  ...   ms║  3000  ms║
╠══════════╬══════════╬══════════╬══════════╬══════════╣
║     ↑    ║          ║          ║          ║          ║
║ stop task║          ║          ║          ║          ║
╚══════════╩══════════╩══════════╩══════════╩══════════╝
```

##### Stop all reward tasks

```plain
╔══════════════════════════════════════════════════════╗
║             REWARD TASK QUEUE (STOPPING)             ║
╠══════════╦══════════╦══════════╦══════════╦══════════╣
║ >task 001║  task 002║  task 003║  task ...║  task 100║
║  1000  ms║  100   ms║  1200  ms║  ...   ms║  3000  ms║
╠══════════╩══════════╩══════════╩══════════╩══════════╣
║        >>>>>>>>> STOPPING ALL TASK >>>>>>>>>         ║
╚══════════════════════════════════════════════════════╝

╔══════════════════════════════════════════════════════╗
║              REWARD TASK QUEUE (EMPTY)               ║
╠══════════╦══════════╦══════════╦══════════╦══════════╣
║          ║          ║          ║          ║          ║
║          ║          ║          ║          ║          ║
╠══════════╩══════════╩══════════╩══════════╩══════════╣
║       >>>>>>>>> ALL TASKS TERMINATED >>>>>>>>>       ║
╚══════════════════════════════════════════════════════╝
```

#### Data frame

```plain
╔═══════════════════════════════════╗
║             DATA FRAME            ║
╠═══════════╦═══════════╦═══════════╣
║  1 * byte ║  1 * byte ║  4 * byte ║
║  uint8_t  ║  uint8_t  ║  uint32_t ║
╠═══════════╬═══════════╬═══════════╣
║ device id ║  command  ║  payload  ║
╚═══════════╩═══════════╩═══════════╝
```

- `device_id` is a `uint8_t`:
  - range: `0x00` - `0xFF`
  - `0x00` broadcast to all devices
- `cmd` is a `uint8_t`:
  - `0x00` **(START)**: add a new reward task
  - `0x01` **(STOP)**: stop all reward tasks or stop the current task
  - `0x02` **(REVERSE)**: reverse the pump direction
  - `0x03` **(SET_SPEED)**: set pump speed
- `payload` is a `uint32_t` Interpretation depends on cmd:
  - **START**: add a new reward task with a duration, ***unit: ms***
  - **STOP**:
    - `0` : stop all reward tasks
    - `!= 0` : stop the current task
  - **SET_SPEED**: the speed persentage of the pump, range: 0 - 100

#### Freature data frame

```plain
╔═══════════════════════╗
║       DATA FRAME      ║
╠═══════════╦═══════════╣
║  1 * byte ║ 63 * byte ║
║  uint8_t  ║  uint8_t  ║
╠═══════════╬═══════════╣
║ device id ║  payload  ║
╚═══════════╩═══════════╝
```

`device id` is a `uint8_t`:
  - range: `0x00` - `0xFF`
  - `0x00` broadcast to all devices

`payload` is a list of `uint8_t`: 63 bytes, feature data payload. feature data frame includes various data structures, including:
```cpp
union feature_payload_t {
  wifi_info_t wifi_info;
  device_info_t device_info;
  simia::start_mode_t start_mode;
};

struct wifi_info_t
{
  uint8_t ssid_len;
  uint8_t password_len;
  uint8_t ssid[29];
  uint8_t password[30];
};

struct device_info_t{
  uint8_t device_id;
  uint8_t nickname_len;
  uint8_t nickname[30];
};

namespace simia
{
  enum class start_mode_t : uint8_t
  {
    NORMAL = 0x00,
    FLASH = 0x01,
    ACTIVE_OTA = 0x02,
  };
};
```