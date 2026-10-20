# TEMP-IKNET — DS18B20 no GPIO38

ESP32-S3 com ESP-IDF 5.3–5.x. Apenas sensor e logs no terminal via ESP_LOGI.

## Ligacao

- DS18B20 VCC/VDD: 3V3.
- DS18B20 GND: GND.
- DS18B20 DQ: GPIO38.
- Resistor de 4,7 kohms entre DQ e 3V3 (alimentacao de tres fios).

## Configurar e executar

```powershell
cd D:/pnaat/temperatura/temp-iknet
idf.py reconfigure
idf.py menuconfig
idf.py build
idf.py -p COM5 flash monitor
```

Use a porta real em lugar de COM5. No menu IKNET - Sensor de temperatura,
confira GPIO de dados = 38. Em Component config / Log output, selecione Info.
O canal de console deve corresponder ao conector usado: UART0 para USB-UART
ou USB Serial/JTAG para USB nativo. A configuracao existente do console foi mantida.

Exemplo: `I (...) IKNET: Temperatura: 28.50 C | Nivel: NORMAL`.
Frio <21; normal 21–40; atencao >40 ate 50; critico >50 graus.
Classificacao imediata de bancada, sem os temporizadores do controle completo.
Leituras invalidas sao avisadas no terminal. O sensor e procurado novamente
apos desconexao. Cada ciclo inclui conversao e espera de um segundo.

Nao utiliza OLED, I2C de tela ou LED RGB. Copias antigas .bak nao entram na compilacao.
Compilacao ESP-IDF e teste fisico pendentes: a tentativa anterior de ativar o
ESP-IDF nao encontrou o ambiente virtual Python.
