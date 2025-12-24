idf_component_register(
                       SRCS 
                        "main.cpp"
                        "SPI/SPI-master.cpp"
                        "SPI/SPI-Slave.cpp"
                        "SmartPointers/DMASmartPointer.cpp"
                       INCLUDE_DIRS 
                        "."
                        "SPI"
                        "SmartPointers"
                       REQUIRES esp_driver_spi
                       REQUIRES esp_timer
                       )
