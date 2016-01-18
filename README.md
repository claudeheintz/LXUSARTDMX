# LXUSARTDMX
DMX Driver for Arduino/AVR microcontrollers

   LXUSARTDMX is a driver for sending or receiving DMX using an AVR microcontroller's USART
   
   LXUSARTDMX output mode continuously sends DMX once its interrupts have been enabled using startOutput().
   Use setSlot() to set the level value for a particular DMX dimmer/address/channel.
   
   LXUSARTDMX input mode continuously receives DMX once its interrupts have been enabled using startInput()
   Use getSlot() to read the level value for a particular DMX dimmer/address/channel.
   
   LXUSARTDMX is used with a single instance called LXSerialDMX	