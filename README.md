# Vehicle Speed Tracker - Zephyr RTOS

## Alunos
- Gabriel Lucas Bento Germano
- Maria Fernanda B. P. Costa

## Descrição do Projeto

Este projeto implementa um sistema de rastreamento e monitoramento de velocidade veicular utilizando o Zephyr RTOS. O sistema é capaz de detectar a velocidade de veículos em tempo real através de sensores magnéticos, registrar infrações de velocidade e transmitir os dados coletados para um serviço web na nuvem.

### Características Principais

- **Detecção de Velocidade**: Utiliza dois sensores magnéticos para calcular a velocidade dos veículos
- **Sincronização de Tempo**: Integração com servidor SNTP para timestamp preciso
- **Transmissão de Dados**: Cliente HTTP para envio de dados de infrações para servidor remoto
- **Configuração Flexível**: Sistema totalmente configurável via Kconfig
- **Modo Simulação**: Capacidade de simular o funcionamento para testes e desenvolvimento
- **Validação de Dados**: Sistema de validação de placas veiculares

## Arquitetura do Sistema

### Módulos Principais

#### 1. **Camera Handler** (`camera_handler.c`)
Responsável pelo gerenciamento da captura de imagens e geração das hashs.

#### 2. **Data HTTP Client** (`data_http_client.c`)
Implementa o cliente HTTP para transmissão de dados de infrações para o servidor remoto.

#### 3. **Display Controller** (`display.c`)
Controla a interface de display para visualização das infrações capturadas pelo sistema.

#### 4. **Main Controller** (`main.c`)
Controlador principal que coordena todos os módulos do sistema.

#### 5. **RTC Handler** (`rtc.c`)
Gerencia o relógio de tempo real e sincronização via SNTP.

#### 6. **Sensors Controller** (`sensors.c`)
Controla os sensores utilizados para detecção de velocidade.

#### 7. **Plate Validator** (`validate_plate.c`)
Implementa algoritmos de validação de placas veiculares brasileiras.

#### 8. **Web Server** (`server.py`)
Servidor web Python para recebimento e processamento dos dados transmitidos.

### Arquitetura de Threads

O sistema utiliza múltiplas threads para operação concorrente:

- **Thread Principal**: Coordenação geral do sistema
- **Thread de Sensores**: Monitoramento contínuo dos sensores magnéticos
- **Thread HTTP**: Transmissão assíncrona de dados
- **Thread SNTP**: Sincronização periódica de tempo
- **Thread de Display**: Atualização da interface visual

## Configuração do Ambiente

### Pré-requisitos

- **Zephyr SDK** versão 0.17.0 ou superior
- **Python 3.8+** para o servidor web
- **QEMU** para emulação (opcional)
- **West** (Zephyr's meta-tool)

### Instalação do Zephyr SDK

```bash
# Baixar e instalar o Zephyr SDK
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.17.0_linux-x86_64.tar.gz
tar xvf zephyr-sdk-0.17.0_linux-x86_64.tar.gz
cd zephyr-sdk-0.17.0
./setup.sh
```

### Configuração do Workspace

```bash
# Clonar o repositório
git clone <url-do-repositorio>
cd vehicle-speed-tracker

# Inicializar o workspace West
west init -l .
west update

# Instalar dependências Python
pip install -r requirements.txt
```

## Execução no QEMU

### Compilação e Execução

```bash
# Configurar o projeto
west build -b qemu_cortex_m3 .

# Executar no QEMU
west build -t run
```

## Configuração via Kconfig

O sistema oferece amplas opções de configuração através do Kconfig. Para acessar o menu de configuração:

```bash
west build -t menuconfig
```

### Principais Opções Configuráveis

#### Detecção de Velocidade Veicular

- **`VEHICLE_SPEED_DETECTION`**: Habilita/desabilita a detecção de velocidade
- **`RADAR_SENSOR_DISTANCE_MM`**: Distância entre sensores (100-1000mm)
- **`RADAR_SPEED_LIMIT_KMH`**: Limite máximo de velocidade (10-300 km/h)
- **`SENSOR_DEBOUNCE_MS`**: Tempo de debounce dos sensores (10-500ms)

#### Configuração de Threads

- **`THREAD_STACK_SIZE`**: Tamanho máximo da pilha das threads
- **`THREAD_PRIORITY`**: Prioridade das threads (0-10)

#### Configuração SNTP

- **`DEFAULT_SNTP_SERVER`**: Servidor SNTP padrão (default: 200.160.7.186)
- **`TIME_SYNC_INTERVAL_MS`**: Intervalo de sincronização (1min-24h)
- **`NUMBER_OF_RETRIES`**: Número de tentativas para requisições SNTP

#### Cliente HTTP

- **`SERVER_HOSTNAME`**: Hostname do servidor remoto
- **`SERVER_PORT`**: Porta do servidor (default: 5000)
- **`SERVER_URL`**: URL da API (default: /api/speed-infractions)
- **`CLIENT_MAX_RECV_BUF_LEN`**: Tamanho máximo do buffer de recepção
- **`RECV_TIMEOUT_MS`**: Timeout para recepção de dados

#### Modo Simulação

- **`SYSTEM_SIMULATION`**: Habilita modo de simulação
- **`SIMULATION_SPEED`**: Velocidade simulada do veículo (0-300 km/h)

## Funcionamento dos Módulos

### Detecção de Velocidade

O sistema utiliza dois sensores magnéticos posicionados a uma distância conhecida. Quando um veículo passa sobre os sensores:

1. O primeiro sensor detecta a presença do veículo
2. O segundo sensor confirma a passagem
3. O tempo entre as detecções é calculado
4. A velocidade é determinada usando: `Velocidade = Distância / Tempo`

### Sincronização de Tempo

O módulo RTC sincroniza periodicamente com um servidor SNTP para manter a precisão temporal:

1. Conecta ao servidor SNTP configurado
2. Solicita timestamp atual
3. Ajusta o RTC interno
4. Agenda próxima sincronização

### Transmissão de Dados

Quando uma infração é detectada:

1. Os dados são formatados em JSON
2. Uma requisição HTTP POST é enviada ao servidor
3. A resposta é validada
4. Em caso de falha, nova tentativa é agendada

### Validação de Placas

O sistema implementa validação para placas brasileiras:

- **Formato Antigo**: ABC-1234
- **Formato Mercosul**: ABC1D23
- Verificação de dígitos válidos
- Validação de padrões regionais

## Servidor Web na Nuvem

### Configuração do Servidor Python

O servidor web (`server.py`) implementa uma API REST para recebimento de dados:

```bash
# Executar o servidor
python scripts/server.py
```

### Endpoints Disponíveis

- **POST /api/speed-infractions**: Recebe dados de infrações

### Exemplo de Payload JSON

```json
{
    "timestamp": "28/05/2025 10:30:00",
    "plate": "ABC1234",
    "speed": 90,
    "country": "BR"
}
```

## Testes

### Testes Automatizados

```bash
# Executar testes unitários
west build -t test

# Executar testes de integração
python -m pytest tests/
```

### Testes Manuais

1. **Teste de Simulação**:
   - Habilitar `SYSTEM_SIMULATION`
   - Configurar `SIMULATION_SPEED`
   - Verificar detecção e transmissão

2. **Teste de Conectividade**:
   - Verificar conexão SNTP
   - Testar envio de dados HTTP
   - Validar recepção no servidor

3. **Teste de Sensores** (hardware real):
   - Calibrar distância entre sensores
   - Testar debounce
   - Verificar precisão de velocidade


## Exemplo de Uso

### Configuração Típica para Rodovia

```kconfig
CONFIG_VEHICLE_SPEED_DETECTION=y
CONFIG_RADAR_SENSOR_DISTANCE_MM=500
CONFIG_RADAR_SPEED_LIMIT_KMH=110
CONFIG_SENSOR_DEBOUNCE_MS=100
CONFIG_DEFAULT_SNTP_SERVER="pool.ntp.br"
CONFIG_SERVER_HOSTNAME="api.traffic-monitor.com"
```

### Configuração para Área Urbana

```kconfig
CONFIG_VEHICLE_SPEED_DETECTION=y
CONFIG_RADAR_SENSOR_DISTANCE_MM=300
CONFIG_RADAR_SPEED_LIMIT_KMH=60
CONFIG_SENSOR_DEBOUNCE_MS=50
CONFIG_TIME_SYNC_INTERVAL_MS=1800000
```

## Contribuição

Para contribuir com o projeto:

1. Fork o repositório
2. Crie uma branch para sua feature
3. Implemente as modificações
4. Execute os testes
5. Submeta um Pull Request

## Licença

Este projeto está licenciado sob a [Licença Apache 2.0](LICENSE).

## Suporte

Para suporte técnico ou dúvidas:

- Abra uma issue no repositório
- Consulte a documentação do Zephyr RTOS
- Verifique os logs de debug do sistema

---

**Desenvolvido com Zephyr RTOS para a disciplina de Sistemas Embarcados na Universidade Federal de Alagoas.**
