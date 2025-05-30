# Hurricane IoT Solution

## Estrutura do Repositório

```
gs_iot/
├─ README.md                
├─ node-red/
│   └─ flow.json             
├─ furacao/
│   ├ platformio.ini
│   ├ wokwi.toml
│   ├ diagram.json
│   └ src/main.cpp           
├─ possivel_furacao/
│   └ (mesmos arquivos com alterações na lógica apenas)      
├─ sem_furacao/
│   └ (mesmos arquivos com alterações na lógica apenas)      
└─ aleatorio/
    └ (mesmos arquivos com alterações na lógica apenas)      
```

## Visão Geral

Cada pasta representa um projeto PlatformIO + Wokwi que simula um ESP32 virtual publicando leituras de temperatura, umidade e pressão em diferentes cenários:

* **furacao/**: valores extremos (temperatura 30–40 °C, umidade 90–100 %, pressão 900–960 hPa)
* **possivel\_furacao/**: alerta (26–32 °C, 80–90 %, 960–980 hPa)
* **sem\_furacao/**: normal (20–28 °C, 50–70 %, 980–1020 hPa)
* **aleatorio/**: a cada ciclo escolhe aleatoriamente um dos três modos acima

Todas as instâncias publicam via MQTT no tópico `hurricane/<DEVICE_ID>/data` do broker HiveMQ Cloud.

## Pré-requisitos

* **VS Code** com as extensões:

  * PlatformIO IDE
  * Wokwi Arduino Simulator (Wokwi.wokwi-vscode)
* **Node.js** (>= v12) e **npm**
* **Node-RED** instalado (`npm install -g node-red`)
* Conta gratuita no **ThingSpeak** ([https://thingspeak.com](https://thingspeak.com))
* Conta gratuita no **HiveMQ Cloud** ([https://www.hivemq.cloud](https://www.hivemq.cloud))

## Configuração de Credenciais

1. **Wi-Fi**

   * SSID e senha: definidos em `src/main.cpp` de cada projeto (`WIFI_SSID`, `WIFI_PASS`).
2. **HiveMQ Cloud**

   * Broker URL, porta 8883, usuário e senha em cada `main.cpp` e no node MQTT do Node-RED.
3. **ThingSpeak**

   * Crie um canal com **4 campos**:

     1. `temperature`
     2. `humidity`
     3. `pressure`
     4. `sensorType`
   * Anote o **Channel ID** e **Write API Key**, e atualize:

     ```cpp
     #define THINGSPEAK_CHANNEL_ID  <SEU_CHANNEL_ID>
     #define THINGSPEAK_API_KEY    "<SUA_WRITE_API_KEY>"
     ```

## Como Executar

### 1. Simulação Wokwi (por sensor)

Para cada pasta **furacao**, **possivel\_furacao**, **sem\_furacao** e **aleatorio**:

1. Abra apenas aquela pasta no VS Code (File → Open Folder).
2. Pressione **F1** → `PlatformIO: Build` para compilar.
3. Abra `diagram.json` e clique em **▶ Simulate in Wokwi** (code lens sobre `parts`).
4. No Serial Monitor, confirme mensagens:

   ```
   Conectando WiFi...WiFi OK
   Conectando MQTT... conectado!
   Publicado em hurricane/<DEVICE_ID>/data: {...}
   ```

### 2. Submeter ao ThingSpeak via Node-RED

1. Inicie o Node-RED:

   ```bash
   node-red
   ```
2. No navegador, acesse `http://127.0.0.1:1880`.
3. Importe o fluxo em `node-red/flow.json` (☰ → Import → Clipboard).
4. Configure o node **MQTT**:

   * Broker: sua URL HiveMQ Cloud
   * Porta: `8883`, TLS habilitado
   * Usuário/Senha: suas credenciais
5. Clique em **Deploy**.
6. No painel **Debug**, observe `msg.statusCode: 200` a cada mensagem.

### 3. Verificar no ThingSpeak

1. Acesse seu canal e abra a **Private View**.
2. Veja os gráficos de Field 1–4 atualizando com os dados simulados.
3. Aguarde até \~20 s para os pontos aparecerem.

## Fluxo Node-RED

Arquivo: `node-red/flow.json`:

```json
[ ... fluxo MQTT in → parse JSON → function monta URL → HTTP request → debug ... ]
```

Você pode editar o **function** para ajustar o mapeamento de `sensorType` (1=sem, 2=possível, 3=furacão).

## Integrantes
#### Enzo Oliveira RM 553185
#### Arthur Fenili RM 552752
#### Vinicio Rapahel RM 553813

