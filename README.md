Firmware Baseado em ESP-IDF

Este repositório contém o desenvolvimento do firmware para uma Estação IoT Interativa projetada para a placa **Franzininho WiFi LAB01** (baseada no ESP32-S2). O objetivo principal do projeto é monitorar variáveis ambientais em tempo real utilizando os princípios de sistemas operacionais de tempo real (RTOS) e comunicação em nuvem via protocolo MQTT.

Arquitetura do Sistema e Modularização

Visando a reutilização de código e a organização do projeto, o firmware foi totalmente estruturado em componentes independentes de hardware e software dentro do diretório `components/`. A comunicação entre esses blocos é feita de forma síncrona e assíncrona utilizando recursos nativos do kernel do FreeRTOS.

Componentes Criados:
* **`wifi_manager`**: Gerencia a conectividade Wi-Fi operando exclusivamente no modo Station (STA). Possui um manipulador de eventos dedicado para realizar tentativas de reconexão automática em segundo plano caso a queda do sinal seja detectada.
* **`storage_nvs`**: Responsável pela persistência de dados na memória Flash (Non-Volatile Storage). Controla a leitura/escrita da estrutura de limites de alarme e gerencia a gravação e paginação de logs de eventos com carimbo de data/hora.
* **`sensors`**: Abstrai a coleta de dados de sensores. Cria uma tarefa periódica no FreeRTOS que realiza a leitura analógica do sensor de luminosidade (LDR) e simula as leituras de temperatura e umidade (DHT11), despachando os dados para o restante do sistema.
* **`display_ui`**: Responsável pela interface com o usuário. Simula a atualização de menus de monitoramento em uma tela OLED e configura interrupções de hardware externas (ISR) acopladas a botões físicos para silenciar o Buzzer de alerta de forma imediata.
* **`mqtt_client_iot`**: Gerencia a pilha do protocolo MQTT para conexão assíncrona com o broker do **Adafruit IO**. Trata a publicação periódica de telemetrias e fica à escuta (subscrição) do feed remoto para atuação no LED RGB.

Gerenciamento de Tarefas e Concorrência (FreeRTOS)

O núcleo da aplicação executa de forma concorrente gerenciado pelo escalonador do FreeRTOS. A divisão de tarefas, prioridades e mecanismos de sincronização foram desenhados da seguinte forma:

1.  **`vTaskSensors` (Prioridade 5 - Periódica)**: Desperta rigorosamente a cada 5 segundos utilizando `vTaskDelayUntil()`, realiza a leitura dos sensores e utiliza `xQueueOverwrite()` para enviar o pacote de dados estruturado à fila.
2.  **`vTaskAlarmManager` (Prioridade 6 - Bloqueante / Alta Prioridade)**: Fica em estado de bloqueio aguardando novos dados na fila através de `xQueueReceive()`. Assim que liberada, compara os valores com os limites carregados da memória NVS. Caso detecte uma violação, aciona o sinal de alerta e gera a string de log que é salva na partição Flash. Também realiza as publicações MQTT.
3.  **`vTaskDisplayKeyboard` (Prioridade 5 - Contínua)**: Atualiza as informações visuais da estação e monitora se a flag de alarme está ativa. Caso o semáforo binário de silenciamento seja entregue, desliga o Buzzer de forma síncrona.
4.  **`vTaskSerialConsole` (Prioridade 2 - Baixa Prioridade)**: Monitora a entrada de caracteres via UART de forma assíncrona. Permite comandos rápidos de gerenciamento pelo terminal.

Comunicação Inter-Processos (IPC):
* **Fila (`sensor_queue`)**: Utilizada para transferir de forma segura a estrutura contendo os dados de temperatura, umidade e luz do componente de sensores para a tarefa de gerenciamento central.
* **Semáforo Binário (`buzzer_silence_sem`)**: Libera a tarefa de controle a partir de uma Interrupção de Hardware (ISR) gerada pelo botão físico do teclado, garantindo tempo de resposta imediato para o silenciamento do alarme.



 Console Serial e Interface de Comando

Para auditoria e testes do sistema, a console UART expõe um menu interativo de baixa prioridade que responde aos seguintes comandos digitados no terminal do Monitor:

 `l` ou `L`: Lê e lista todo o histórico de logs de alarmes gravados permanentemente na partição NVS da memória Flash.
 `c` ou `C`: Limpa completamente o histórico de logs guardado na partição, resetando os contadores internos.

---

Como Compilar e Executar

Certifique-se de ter o ambiente do ESP-IDF instalado e configurado em seu terminal de comando.

1. Escolha o target correto para a placa Franzininho:
   bash
   idf.py set-target esp32s2
