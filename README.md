# CP5 — Vinheria Agnello: ESP32 + MQTT

Projeto desenvolvido para a disciplina **Edge Computing & Computer Systems**, da FIAP. O objetivo é conectar um ESP32 a um servidor IoT utilizando sensores ambientais e o protocolo MQTT.

O sistema realiza a leitura de temperatura, umidade e luminosidade, publica os dados no servidor e permite controlar o LED integrado à placa por meio de comandos MQTT.

## Integrantes

| Nome | RM |
|---|---|
| Mateus Alves Borges | 571424 |
| Arthur de Sá Lopes | 569526 |
| Matheus Carvalho | 569454 |

## Objetivo do projeto

Desenvolver um sistema de monitoramento ambiental para a Vinheria Agnello utilizando um ESP32.

O dispositivo coleta dados dos sensores e os envia ao servidor IoT por MQTT. Também recebe comandos remotos para ligar e desligar o LED integrado à placa.

## Componentes utilizados

| Componente | Função |
|---|---|
| ESP32 | Processamento e comunicação Wi-Fi/MQTT |
| DHT11 | Leitura de temperatura e umidade |
| LDR | Leitura de luminosidade |
| LED integrado | Controle remoto por MQTT |
| Protoboard e jumpers | Conexão dos componentes |

## Tecnologias utilizadas

- Arduino IDE e C++.
- ESP32 e conexão Wi-Fi.
- Protocolo MQTT.
- Biblioteca PubSubClient.
- Biblioteca DHT Sensor Library, da Adafruit.
- Aplicativo MyMQTT.
- Servidor IoT compatível com a configuração MQTT utilizada.

## Requisitos

- Placa ESP32 compatível com o código.
- Sensor DHT11.
- Sensor LDR com circuito divisor de tensão.
- Cabo USB para programação.
- Arduino IDE com suporte à placa ESP32.
- Bibliotecas `PubSubClient` e `DHT sensor library`.
- Rede Wi-Fi disponível.
- Servidor MQTT acessível.
- Aplicativo MyMQTT ou outro cliente MQTT.

## Montagem dos componentes

As conexões utilizadas no código são:

| Componente | Pino do ESP32 |
|---|---|
| DHT11 — DATA | GPIO 4 |
| LDR — saída analógica | GPIO 34 |
| LED integrado | GPIO 2 |

O DHT11 deve ser conectado à alimentação, ao GND e ao GPIO 4. O LDR deve utilizar um circuito divisor de tensão, com a saída analógica conectada ao GPIO 34.

**Observação:** confira a pinagem e a alimentação dos componentes de acordo com o modelo da placa e o kit utilizado.

## Configuração do ambiente

Antes de executar o projeto, abra o código-fonte e configure os parâmetros de acordo com sua própria rede e seu servidor MQTT.

| Variável | O que configurar |
|---|---|
| `WIFI_SSID` | Nome da rede Wi-Fi |
| `WIFI_SENHA` | Senha da rede Wi-Fi |
| `BROKER_IP` | Endereço IP ou hostname do broker MQTT |
| `BROKER_PORTA` | Porta utilizada pelo broker |
| `API_KEY` | Identificador ou chave exigida pelo servidor |
| `DEVICE_ID` | Identificador do dispositivo cadastrado no servidor |

Os valores presentes no código são exemplos e devem ser substituídos pelos dados do ambiente utilizado.

A porta `1883` está definida como padrão para conexões MQTT sem TLS. Utilize a porta exigida pelo servidor configurado.

**Importante:** não publique senhas reais, chaves de acesso ou outras informações sensíveis no repositório. Em projetos maiores, prefira manter as configurações privadas fora do código versionado.

### Bibliotecas necessárias

No Gerenciador de Bibliotecas do Arduino IDE, instale:

- `PubSubClient`, de Nick O'Leary.
- `DHT sensor library`, da Adafruit.

A biblioteca `WiFi.h` faz parte do suporte de placas ESP32.

## Passo a passo para execução

### 1. Instalar o Arduino IDE

Instale o [Arduino IDE](https://www.arduino.cc/en/software) e configure o suporte à placa ESP32.

### 2. Instalar as bibliotecas

Instale `PubSubClient` e `DHT sensor library` pelo Gerenciador de Bibliotecas.

### 3. Montar o circuito

Conecte o DHT11 e o LDR de acordo com a tabela de montagem.

### 4. Configurar a conexão

Preencha as configurações de Wi-Fi e MQTT no código. Confira se o dispositivo está cadastrado no servidor e se os tópicos esperados correspondem aos identificadores utilizados.

### 5. Enviar o código para a placa

Conecte o ESP32 ao computador por USB, selecione a placa e a porta serial corretas e clique em **Upload**.

### 6. Verificar o funcionamento

Abra o Monitor Serial com a velocidade de `115200 baud`.

O programa deve informar o andamento da conexão Wi-Fi, da conexão MQTT e da publicação dos dados.

## Comunicação MQTT

Os tópicos são construídos automaticamente com base nos valores de `API_KEY` e `DEVICE_ID`.

A estrutura utilizada é:

| Tópico | Finalidade |
|---|---|
| `/<API_KEY>/<DEVICE_ID>/attrs` | Publicação dos dados dos sensores |
| `/<API_KEY>/<DEVICE_ID>/cmd` | Recebimento de comandos para o LED |
| `/<API_KEY>/<DEVICE_ID>/cmdexe` | Confirmação da execução dos comandos |

Substitua os marcadores pelos valores configurados no código. O formato exato dos tópicos deve ser compatível com o servidor MQTT utilizado.

### Publicação dos dados

A cada cinco segundos, o ESP32 lê os sensores e publica os resultados no tópico de atributos em formato JSON.

Exemplo de mensagem:

```json
{
  "temperatura": 12.5,
  "umidade": 60.0,
  "luminosidade": 25
}
```

Os valores são ilustrativos e representam:

- `temperatura`: temperatura em graus Celsius.
- `umidade`: umidade relativa do ar em porcentagem.
- `luminosidade`: valor convertido para uma escala de 0 a 100.

O LDR realiza cinco leituras para calcular uma média e reduzir oscilações.

### Recebimento de comandos

O dispositivo recebe comandos pelo tópico:

```text
/<API_KEY>/<DEVICE_ID>/cmd
```

| Mensagem | Ação |
|---|---|
| `on` | Liga o LED |
| `off` | Desliga o LED |

Após reconhecer um comando, o ESP32 envia uma confirmação ao tópico:

```text
/<API_KEY>/<DEVICE_ID>/cmdexe
```

O formato da confirmação é:

```text
<DEVICE_ID>@on|Ok
```

Para o comando de desligamento, a resposta será:

```text
<DEVICE_ID>@off|Ok
```

Os comandos devem ser publicados com o payload esperado e nos tópicos correspondentes.

## Configuração do MyMQTT

1. Abra o aplicativo MyMQTT ou outro cliente MQTT.
2. Configure o endereço e a porta do mesmo broker utilizado pelo ESP32.
3. Inscreva-se no tópico de atributos para acompanhar as leituras.
4. Publique `on` ou `off` no tópico de comandos.
5. Verifique o acionamento do LED e a mensagem de confirmação.

O ESP32 e o cliente MQTT precisam utilizar o mesmo servidor e os tópicos corretos.

## Limites de referência ambiental

O código contém valores de referência que podem ser ajustados conforme os requisitos do projeto.

| Parâmetro | Faixa ideal | Limites de referência para alerta |
|---|---|---|
| Temperatura | 10 °C a 15 °C | 8 °C a 18 °C |
| Umidade | 50% a 70% | 40% a 80% |
| Luminosidade | Até 30% | Acima de 30% até 70% |

Os limites estão definidos no código para avaliação dos valores lidos. Na versão atual, a classificação calculada não é enviada ao servidor nem exibida no Monitor Serial; o payload publicado contém somente as leituras dos sensores.

## Testes e validação

Para validar o projeto:

1. Verifique a conexão do ESP32 ao Wi-Fi.
2. Confirme a conexão com o broker MQTT.
3. Confira as leituras do DHT11.
4. Observe a variação dos valores do LDR.
5. Verifique a publicação dos dados no tópico de atributos.
6. Envie os comandos `on` e `off`.
7. Confirme o acionamento do LED e a resposta no tópico de execução.

## Problemas comuns

| Problema | Possível causa | Solução |
|---|---|---|
| Não conecta ao Wi-Fi | Nome da rede ou senha incorretos | Confira as configurações |
| Não conecta ao MQTT | Endereço, porta ou servidor incorretos | Verifique a configuração e a disponibilidade do broker |
| DHT11 sem leitura | Ligação incorreta ou falha do sensor | Confira alimentação, GND e DATA |
| Luminosidade invertida | Montagem do divisor de tensão ou conversão inadequada | Confira o circuito e ajuste a leitura |
| Mensagens não aparecem no cliente MQTT | Tópicos ou broker diferentes | Confira as configurações dos dois clientes |
| LED não responde | Comando ou tópico incorreto | Publique exatamente `on` ou `off` no tópico de comandos |

## Segurança

As configurações do projeto utilizam valores de exemplo para permitir que cada pessoa configure seu próprio ambiente. Não inclua senhas reais de Wi-Fi, chaves privadas ou credenciais de acesso no repositório público.

## Referências

- [Arduino IDE](https://www.arduino.cc/en/software)
- [PubSubClient](https://github.com/knolleary/pubsubclient)
- [DHT Sensor Library — Adafruit](https://github.com/adafruit/DHT-sensor-library)
- [Wokwi](https://wokwi.com/)
- [FIWARE](https://www.fiware.org/)
- [Repositório do projeto](https://github.com/TheMathh/checkpoint-5-edgecomputing-1ESPQ)