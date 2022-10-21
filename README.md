# Mavsdk Gimbal Service Implementation

```mermaid
graph TD;
    Gimbal-->UART;
    UART-->Gimbal;
    UART-->gremsy_mavsdk;
    gremsy_mavsdk-->UART;
    gremsy_mavsdk-->MAVSDK_Gimbal_Client;
    MAVSDK_Gimbal_Client-->gremsy_mavsdk;
```