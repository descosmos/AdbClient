# Class Graph


```c++

AdbClient(class)
|
|---- adbCommand_(member)
|


AdbCommand(class)
|
|---- m_tcp_client(member)
|
|---- m_tcp_server(member)
|
|- - - - - - HostCommand(class)
                 |
                 |---- execute_cmd(method)
                 |
                 |---- get_version(method)
                 |
                 |---- get_devices(method)
                 |
                 |---- get_devices_with_path(method)
                 |
                 |---- track_deivices(method)
                 |
                 |---- kill(method)
                 |
                 |---- connect(method)
                 |
                 |---- disconnect(method)
|
|
|
|- - - - - - HostSerialCommand(class)
                 |
                 |---- execute_cmd(method)
                 |
                 |---- forward(method)
                 |
                 |---- list_forward(method)
                 |
                 |---- kill_forward(method)
                 |
                 |---- kill_forward_all(method)
                 |
                 |---- get_device_path(method)
                 |
                 |---- get_serial_no(method)
                 |
                 |---- get_state(method)
|
|
|- - - - - - LocalCommand(class)
                 |
                 |---- transport(method)
                 |
                 |---- shell(method)
                 |
                 |---- sync(method)
                 |
                 |---- screencap(method)
                 |
                 |---- list_packages(method)
                 |
                 |---- tcpip(method)
                 |
                 |---- usb(method)
                 |
                 |---- logcat(method)
                 |
                 |---- get_properties(method)
                 |
                 |---- root(method)
                 |
                 |---- reverse(method)
                 |
                 |---- list_reverse(method)
                 |
                 |---- kill_reverse(method)
                 |
                 |---- kill_reverse_all(method)
|
|



```