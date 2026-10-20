# CP5 — Vinheria Agnello: ESP32 + MQTT

Projeto desenvolvido para a disciplina **Edge Computing & Computer Systems**, da FIAP. O objetivo é conectar o projeto da Vinheria Agnello ao servidor IoT utilizando um ESP32, sensores ambientais e o protocolo MQTT.

## Integrantes

| Nome | RM |
|---|---|
| Mateus Alves Borges | 571424 |
| Arthur de Sá Lopes | 569526 |
| Matheus Carvalho | 569454 |

## Objetivo do projeto

Desenvolver um sistema de monitoramento ambiental para a Vinheria Agnello, utilizando sensores conectados ao ESP32 para coletar dados de temperatura, umidade e luminosidade.

Os dados são enviados ao servidor IoT por meio do protocolo MQTT. O sistema também permite o envio de comandos remotos pelo aplicativo MyMQTT para controlar o LED integrado à placa ESP32.

## Componentes utilizados

| Componente | Função |
|---|---|
| ESP32 | Processamento dos dados e comunicação Wi-Fi/MQTT |
| DHT11 | Leitura de temperatura e umidade |
| LDR | Leitura de luminosidade |
| LED integrado | Controle remoto por comandos MQTT |
| Protoboard e jumpers | Conexão dos componentes |

## Tecnologias utilizadas

- Arduino IDE e C++.
- ESP32 e conexão Wi-Fi.
- Protocolo MQTT.
- Biblioteca PubSubClient.
- Biblioteca DHT Sensor Library, da Adafruit.
- Aplicativo MyMQTT.
- Servidor IoT FIWARE.

## Requisitos

- Placa ESP32 compatível com o código.
- Sensor DHT11.
- Sensor LDR com circuito divisor de tensão.
- Cabo USB para programação da placa.
- Arduino IDE com o suporte de placas ESP32 instalado.
- Bibliotecas `PubSubClient` e `DHT sensor library`.
- Acesso à rede Wi-Fi configurada no código.
- Broker MQTT disponível no servidor IoT.
- Aplicativo MyMQTT instalado em um dispositivo conectado à rede.

## Montagem dos componentes

As conexões definidas no código são:

| Componente | Pino do ESP32 |
|---|---|
| DHT11 — DATA | GPIO 4 |
| LDR — saída analógica | GPIO 34 |
| LED integrado | GPIO 2 |

O DHT11 deve ser conectado à alimentação, ao GND e ao GPIO 4. O LDR deve ser conectado a um circuito divisor de tensão, com a saída analógica ligada ao GPIO 34.

**Observação:** confira a pinagem e a alimentação dos componentes de acordo com o kit utilizado.

## Configuração do código

Antes de enviar o código para o ESP32, confira as configurações de Wi-Fi e MQTT:

| Variável | Descrição |
|---|---|
| `WIFI_SSID` | Nome da rede Wi-Fi |
| `WIFI_SENHA` | Senha da rede Wi-Fi |
| `BROKER_IP` | Endereço IP do servidor MQTT |
| `BROKER_PORTA` | Porta do broker MQTT: `1883` |
| `API_KEY` | Identificação da API: `TEF` |
| `DEVICE_ID` | Identificação do dispositivo: `vinheria571424` |

As configurações precisam corresponder ao ambiente disponibilizado para a atividade.

### Bibliotecas necessárias

No Arduino IDE, acesse o Gerenciador de Bibliotecas e instale:

- `PubSubClient`, de Nick O'Leary.
- `DHT sensor library`, da Adafruit.

A biblioteca `WiFi.h` já faz parte do pacote de suporte de placas ESP32.

## Passo a passo para execução

### 1. Instalar e configurar o Arduino IDE

Instale o [Arduino IDE](https://www.arduino.cc/en/software) e configure o suporte de placas ESP32.

### 2. Instalar as bibliotecas

Instale as bibliotecas `PubSubClient` e `DHT sensor library` pelo Gerenciador de Bibliotecas.

### 3. Conectar os componentes

Monte o circuito com o ESP32, o DHT11 e o LDR, seguindo a tabela de conexões.

### 4. Configurar o código

Abra o código-fonte e confira o SSID, a senha da rede Wi-Fi, o endereço do broker e a identificação do dispositivo.

### 5. Enviar o código para o ESP32

Conecte a placa ao computador por USB, selecione a placa e a porta serial corretas e clique em **Upload**.

### 6. Verificar o funcionamento

Abra o Monitor Serial com a velocidade de `115200 baud`.

O programa deve apresentar as mensagens de conexão Wi-Fi, conexão MQTT e publicação dos dados dos sensores.

## Comunicação MQTT

O dispositivo utiliza três tópicos para a comunicação com o servidor IoT.

| Tópico | Finalidade |
|---|---|
| `/TEF/vinheria571424/attrs` | Publicação dos dados dos sensores |
| `/TEF/vinheria571424/cmd` | Recebimento de comandos para o LED |
| `/TEF/vinheria571424/cmdexe` | Confirmação da execução dos comandos |

### Publicação dos dados

A cada 5 segundos, o ESP32 realiza a leitura dos sensores e publica os dados no tópico de atributos.

O formato da mensagem é:

```text
t|<temperatura>|h|<umidade>|l|<luminosidade>
```

Exemplo de mensagem:

```text
t|12.5|h|60.0|l|25
```

Os campos representam:

- `t`: temperatura em graus Celsius.
- `h`: umidade relativa do ar em porcentagem.
- `l`: luminosidade convertida para uma escala de 0 a 100.

Os valores apresentados são ilustrativos.

O LDR realiza cinco leituras para calcular uma média e reduzir oscilações nos resultados.

### Recebimento de comandos

O ESP32 recebe comandos pelo tópico:

```text
/TEF/vinheria571424/cmd
```

Os comandos disponíveis são:

| Payload | Ação |
|---|---|
| `on` | Liga o LED |
| `off` | Desliga o LED |

Após reconhecer o comando, o dispositivo publica uma confirmação no tópico `/TEF/vinheria571424/cmdexe`.

Exemplo de confirmação:

```text
vinheria571424@on|Ok
```

## Configuração do MyMQTT

Para testar a comunicação:

1. Abra o aplicativo MyMQTT.
2. Configure a conexão com o mesmo broker MQTT utilizado pelo ESP32.
3. Assine o tópico `/TEF/vinheria571424/attrs` para receber os dados dos sensores.
4. Publique `on` ou `off` no tópico `/TEF/vinheria571424/cmd`.
5. Confira a alteração do LED e a resposta no tópico `/TEF/vinheria571424/cmdexe`.

O ESP32 e o aplicativo precisam utilizar o mesmo broker e os tópicos corretos.

## Monitoramento das condições ambientais

O código possui limites de referência para avaliar as condições do ambiente.

| Parâmetro | Faixa ideal | Faixa de alerta | Condição crítica |
|---|---|---|---|
| Temperatura | 10 °C a 15 °C | De 8 °C a 18 °C, fora da faixa ideal | Abaixo de 8 °C ou acima de 18 °C |
| Umidade | 50% a 70% | De 40% a 80%, fora da faixa ideal | Abaixo de 40% ou acima de 80% |
| Luminosidade | Até 30% | Acima de 30% até 70% | Acima de 70% |

O estado geral é calculado considerando os três parâmetros:

- `OK`: todos os parâmetros estão dentro das respectivas faixas ideais.
- `ALERTA`: pelo menos um parâmetro está fora da faixa ideal, mas dentro dos limites de alerta.
- `CRITICO`: pelo menos um parâmetro ultrapassou os limites de alerta.

A classificação é apresentada no Monitor Serial. No código atual, o payload enviado ao servidor contém os valores dos sensores, sem o texto correspondente ao estado geral.

## Testes e validação

Para validar o funcionamento do projeto, devem ser realizados os seguintes testes:

1. Verificar a conexão do ESP32 à rede Wi-Fi.
2. Confirmar a conexão com o broker MQTT.
3. Conferir as leituras de temperatura e umidade do DHT11.
4. Verificar a variação da luminosidade captada pelo LDR.
5. Confirmar a publicação das mensagens no tópico de atributos.
6. Enviar os comandos `on` e `off` pelo MyMQTT.
7. Verificar o acionamento do LED e as mensagens de confirmação.

## Problemas comuns

| Problema | Possível causa | Solução |
|---|---|---|
| ESP32 não conecta ao Wi-Fi | SSID ou senha incorretos | Conferir as configurações no código |
| MQTT não conecta | Broker indisponível ou IP incorreto | Conferir o endereço, a porta e a disponibilidade do servidor |
| DHT11 não apresenta leituras | Ligação incorreta ou falha do sensor | Verificar alimentação, GND e DATA |
| Luminosidade apresenta valores invertidos | Orientação do divisor de tensão ou conversão inadequada | Conferir a montagem e ajustar a conversão do LDR |
| Mensagens não aparecem no MyMQTT | Tópico incorreto ou broker diferente | Conferir a conexão e os tópicos utilizados |
| LED não responde aos comandos | Payload ou tópico incorreto | Publicar `on` ou `off` no tópico de comandos |

## Segurança

As credenciais de Wi-Fi estão definidas diretamente no código para fins didáticos. Antes de publicar o projeto em um repositório público, remova senhas reais e utilize uma configuração local não versionada.

## Referências

- Repositório do projeto: https://github.com/TheMathh/checkpoint-5-edgecomputing-1ESPQ
- Arduino IDE: https://www.arduino.cc/en/software
- PubSubClient: https://github.com/knolleary/pubsubclient
- DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
- Wokwi: https://wokwi.com/
- FIWARE: https://www.fiware.org/
