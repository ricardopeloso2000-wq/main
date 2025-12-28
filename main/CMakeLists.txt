idf_component_register(
                       SRCS 
                        "main.cpp"
                        "SPI/SPI-master.cpp"
                        "SPI/SPI-Slave.cpp"
                        "SmartPointers/DMASmartPointer.cpp"
                        "Queues/DMAQueue.cpp"
                       INCLUDE_DIRS 
                        "."
                        "SPI"
                        "SmartPointers"
                        "Queues"
                       REQUIRES esp_driver_spi
                       REQUIRES esp_timer
                       REQUIRES esp_driver_gpio
                       )
