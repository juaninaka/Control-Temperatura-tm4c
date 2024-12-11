# Control-Temperatura-tm4c
Control de temperatura y humedad, lenguaje de bajo nivel, tm4c129ncpdt. Uso de teclado matricial, sensor bmp280 con i2c, dht y pwm

Tomar en cuenta que el uso de las cargas (en este caso foco y motor cc) estan conectados a un mosfet, puede ser conectado a un tip a modo de emisor común, de modo que sea un interruptor, usé mosfet dado que me dio mejores resultados y trae integrado diodo para protección de la bobina del motor y proteger la tiva. El código pwm del foco tiene que ser revisado cuidadosamente.
Tomar en cuenta que los sensores son más sensibles a cambio de temperatura alta que a baja, tener paciencia. Verificar la funcionalidad del pwm con osciloscopio.
