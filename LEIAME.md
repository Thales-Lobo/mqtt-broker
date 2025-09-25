# Broker MQTT Simplificado

Este projeto implementa um **broker MQTT básico** em C, desenvolvido como exercício prático para compreender conceitos de **protocolos de aplicação em redes de computadores**. Ele utiliza o protocolo **MQTT 5.0** em sua forma simplificada, permitindo que múltiplos clientes publiquem e se inscrevam em tópicos, trocando mensagens via utilitários como `mosquitto_pub` e `mosquitto_sub`.

**Autor:** Thales Lobo

---

## 📑 Sumário

- [Broker MQTT Simplificado](#broker-mqtt-simplificado)
  - [📑 Sumário](#-sumário)
  - [Visão Geral](#visão-geral)
  - [Estrutura de Diretórios](#estrutura-de-diretórios)
    - [bin/](#bin)
    - [build/](#build)
    - [include/](#include)
    - [src/](#src)
    - [logs/](#logs)
    - [scripts/](#scripts)
    - [analysis/](#analysis)
    - [state/](#state)
  - [Detalhes dos Componentes](#detalhes-dos-componentes)
  - [Compilação e Execução](#compilação-e-execução)
  - [Sistema de Containers para Simulação](#sistema-de-containers-para-simulação)
  - [Medição de Recursos e Métricas](#medição-de-recursos-e-métricas)
  - [Uso com mosquitto\_pub e mosquitto\_sub](#uso-com-mosquitto_pub-e-mosquitto_sub)
  - [Limitações da Implementação](#limitações-da-implementação)

---

## Visão Geral

- Conexões simultâneas de múltiplos clientes.
- Inscrição de clientes em tópicos.
- Publicação de mensagens em tópicos.
- Encaminhamento das mensagens publicadas aos assinantes correspondentes.
- Persistência de estados em arquivos JSON (`state/topics_state.json`).
- Logs diferenciados com possibilidade de habilitar ou desabilitar o **modo debug**.

---

## Estrutura de Diretórios

```
.
├── LICENSE.txt
├── Makefile
├── README.md
├── analysis
│   ├── data
│   │   ├── cpu_metrics.csv
│   │   └── net_metrics.csv
│   ├── images
│   │   ├── cpu_metrics.png
│   │   └── network_metrics.png
│   ├── plot_metrics.py
│   └── requirements.txt
├── bin
├── build
├── docker
│   ├── Dockerfile.broker
│   └── Dockerfile.client
├── include
├── logs
│   └── broker.log
├── scripts
│   ├── collect_metrics.sh
│   ├── entrypoint_client.sh
│   └── launch.sh
├── src
└── state
    └── topics_state.json
```

### bin/
Contém o executável final do broker (`broker`).

### build/
Armazena arquivos objeto intermediários (`.o`) gerados durante a compilação. Limpo e regenerado pelo `make`.

### include/
Arquivos de cabeçalho (.h) que definem as APIs internas entre os módulos, incluindo `config.h` com a flag `DEBUG_ENABLED`.

### src/
Código-fonte modularizado.

### logs/
Contém o arquivo `broker.log` que registra mensagens de diferentes níveis (INFO, WARN, ERROR, DEBUG).

### scripts/
Scripts auxiliares para:
- Lançar clientes e broker com containers.
- Coletar métricas de CPU e rede.

### analysis/
Scripts e arquivos para geração de métricas e gráficos, salvando as imagens em `analysis/images`.

### state/
Persistência de tópicos e mensagens em `topics_state.json`.

---

## Detalhes dos Componentes

- **main.c:** Inicializa sockets, aceita conexões e chama `broker_handle_client`.
- **broker.c:** Processa pacotes MQTT e envia para os tópicos corretos.
- **client.c:** Estrutura de cliente e envio de mensagens.
- **topic.c:** Gerencia tópicos, assinantes e mensagens.
- **mqtt_parser.c:** Parsing e encoding simplificado de pacotes MQTT.
- **utils.c:** Funções utilitárias, logging e verificação de debug.

---

## Compilação e Execução

Compile e rode facilmente usando `make`:

```bash
# Compila todos os arquivos
make

# Executa o broker com a porta desejada
make run 8000

# Limpa objetos e binários
make clean
```

O binário será gerado em `bin/broker`.

---

## Sistema de Containers para Simulação

Para simular clientes e broker em containers Docker:

1. Crie a bridge de rede para comunicação entre containers:
```bash
docker network create mqtt_net
```

2. Build das imagens:
```bash
docker build -f docker/Dockerfile.broker -t mybroker .
docker build -f docker/Dockerfile.client -t myclient .
```

3. Lançar o broker:
```bash
docker run --name broker --network mqtt_net -p 8000:8000 -v $(pwd)/state:/app/state -v $(pwd)/logs:/app/logs mybroker
```

4. Lançar clientes simulando N clientes conectando a T tópicos:
```bash
docker run -it --rm --name mqtt_clients --network mqtt_net -v $(pwd)/scripts:/app/scripts myclient /app/scripts/entrypoint_client.sh <N> <BROKER_HOST> <BROKER_PORT> <T>
```
**Nota:** Todos os logs são exportados automaticamente para `logs/broker.log`. A execução de múltiplos clientes é configurável via script. Esta simulação é para fins educativos.

---

## Medição de Recursos e Métricas

- O script `collect_metrics.sh` coleta CPU e uso de rede do container broker.
- As métricas são salvas em `analysis/data/cpu_metrics.csv` e `analysis/data/net_metrics.csv`.
- Gráficos são gerados com `plot_metrics.py` em `analysis/images` mostrando média e desvio padrão.

---

## Uso com mosquitto_pub e mosquitto_sub

Para teste simples fora dos containers:
```bash
mosquitto_sub -h localhost -p 8000 -t "teste"
mosquitto_pub -h localhost -p 8000 -t "teste" -m "Olá Mundo!"
```

---

## Limitações da Implementação

- Implementação simplificada do MQTT 5.0.
- Sem autenticação.
- Estrutura de tópicos básica (sem curingas `+` ou `#`).
- Persistência limitada a JSON.
