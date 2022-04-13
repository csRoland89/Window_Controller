# Window_Controller
Elektric window controller
Ver.: 1.0

This controller written in C++ for my rolling shutter. I catch the signals with an RLT-SDR dongle and decode with UniversalRadio Hacker.
The signal is 137 bit length and it can separate to three part:
                                                            - entry part
                                                            - controller part
                                                            - command part

The remote controller has three button:
                                    - command to up-lifting
                                    - command to stop
                                    - command to let down
                                    
The remote controller play the signals four times and if we press the up or the down button, it will send two types of command.

The program has an sqlite database with three data table. the fist one stores the entry signal parts,
the second stores tha controller bits, and the third for tha command bits.

I use two 3rd party library:
                         - sqlite3 api library for open the database file
                         - gpiod library for control the Raspberry GPIO ports

My future goals:
            - rewrite this code in C
            - add multi threding support for the signal loading
            - add options to uplifting the sutters only a part of the total percent
