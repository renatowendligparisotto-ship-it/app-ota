# 📱 Central de Atualização OTA (ESP32 via Firebase & GitHub)

Uma solução completa de interface web progressiva (PWA) para gestão remota e envio de firmwares (Over-The-Air) para placas ESP32, ESP32-S3 e ESP32-C3. O sistema cruza dados do Firebase Realtime Database com a API do GitHub para criar um painel de controlo automatizado e seguro.

## 🚀 Principais Funcionalidades

* **Integração Automática com GitHub:** Lê a API pública do GitHub, filtra ficheiros `.bin` e extrai os links *raw* de download automaticamente (sem necessidade de copiar links manualmente).
* **Filtro Inteligente de Hardware:** Identifica o hardware reportado pela placa no Firebase (ex: ESP32 Clássico, S3 ou C3) e exibe na lista apenas os firmwares compatíveis, prevenindo falhas de hardware por ficheiros incorretos.
* **Instalável (PWA):** Funciona como uma aplicação nativa em Android, iOS, Windows e Linux, operando em janela isolada e guardando dados localmente (Local Storage).
* **Multi-Contas Firebase:** Permite alternar instantaneamente entre diferentes bases de dados (ex: "Casa", "Fábrica", "Chácara") sem precisar de editar o código.
* **Safety Lock (Trava de Segurança):** Bloqueia a interface web durante o envio do comando OTA (timeout de 45s) para evitar sobreposição de comandos e bugs de rede.

## 🏗️ Arquitetura do Sistema

1. **GitHub (Hospedagem de Ficheiros):** Armazena os binários (`.bin`) compilados.
2. **Firebase (Middleware/Telemetria):** Regista o estado da placa (Online/Offline, Uptime, Hardware, Versão Atual) e atua como mensageiro do comando OTA.
3. **PWA (Frontend):** Painel de controlo em HTML/JS puro que interage com o utilizador, lê o GitHub, verifica o Firebase e emite ordens.
4. **ESP32 (Hardware):** Fica em escuta. Ao receber a flag `comando_ota = true` no Firebase, acede ao link injetado, descarrega o ficheiro do GitHub e reinicia com o novo software.

## 🛠️ Como Instalar o Painel PWA

Como este projeto é um PWA puro (Frontend), não necessita de servidores complexos.

1. Aloje os ficheiros `Painel_OTA_PWA.html`, `manifest.json` e `sw.js` através do **GitHub Pages** ou qualquer servidor web simples.
2. Aceda ao link gerado no navegador do seu telemóvel (Chrome/Safari) ou Computador (Chrome/Edge).
3. Na barra de navegação ou através de uma notificação pop-up, clique em **"Instalar Aplicativo"** ou **"Adicionar ao Ecrã Inicial"**.

## 📖 Manual de Operação

### 1. Configurar o Repositório de Firmwares (GitHub)
* No bloco roxo **"Repositório GitHub Automático"**, insira o seu nome de utilizador do GitHub e o nome do repositório onde guarda os ficheiros `.bin`.
* (Opcional) Adicione um "Token de Acesso Pessoal" do GitHub para evitar bloqueios de taxa da API (Erro 403) se atualizar a página mais de 60 vezes por hora.

### 2. Adicionar uma Central (Firebase)
* No bloco azul **"Gerir Contas Firebase"**, introduza um Nome, a sua API Key e o URL da sua Base de Dados do Firebase. 
* Clique em "Guardar e Ligar". Os dados ficam salvos de forma segura na memória do seu navegador.

### 3. Enviar Atualização (OTA)
1. Certifique-se de que a sua placa está ligada à internet e o status no painel mostra **🟢 ONLINE**.
2. No menu principal, selecione a placa que deseja atualizar.
3. O painel irá reportar automaticamente o hardware. Escolha o novo firmware na lista filtrada do GitHub.
4. Clique em **"Atualizar Placa com Selecionado"**. O painel ficará bloqueado durante a transferência (até 45 segundos) e a placa será reiniciada com a nova versão.

## ⚠️ Requisitos do Firmware no ESP32
Para que a placa converse com este painel, o seu código C++ no ESP32 precisa das seguintes bibliotecas instaladas no Arduino IDE:
* **Firebase Arduino Client Library for ESP8266 and ESP32** (Autor: **Mobizt**) - Essencial para ler a flag de OTA e gerir a ligação segura à base de dados.
* **HTTPUpdate** (Embutida no pacote base do ESP32) - Para processar o download do ficheiro `.bin` a partir da API do GitHub.
