cpp_content = """/*
 * =================================================================================
 * PROJETO: Central de Atualização OTA via Firebase e GitHub
 * AUTOR: Renato / Luan
 * 
 * DESCRIÇÃO:
 * Este é o firmware final (Estágio 3) que a placa corre em ambiente de produção.
 * 1. Lê as credenciais Wi-Fi e Firebase salvas na memória interna (Preferences).
 * 2. Conecta-se ao Firebase usando a biblioteca Mobizt.
 * 3. Reporta o seu status e modelo a cada 10 segundos.
 * 4. Fica à escuta da flag 'comando_ota'. Se for true, baixa o ficheiro do GitHub.
 * =================================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

// 1. Bibliotecas do Firebase (Mobizt)
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// 2. Bibliotecas para Download OTA (GitHub exige HTTPS)
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>

Preferences preferences;

// =================================================================================
// CONFIGURAÇÕES DA PLACA (Altere para cada placa física diferente)
// =================================================================================
const String DISPOSITIVO_ID = "Placa_Rele_Sala"; // Nome que aparece no painel web
const String MODELO_HARDWARE = "ESP32_CLASSICO"; // Ex: ESP32_CLASSICO, ESP32_S3_RELE
const String VERSAO_ATUAL = "v1.0.0";            // Atualize isto antes de compilar um novo .bin

// Variáveis de Memória (Lidas do Estágio 1)
String ssid_cliente = "";
String pass_cliente = "";
String apikey_cliente = "";
String url_cliente = "";

// Objetos do Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long tempoAnterior = 0;
const long intervaloReporte = 10000; // 10 em 10 segundos

void executarOTA(String urlDownload);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\\n=== INICIANDO FIRMWARE MESTRE ===");
  Serial.println("Versao: " + VERSAO_ATUAL);

  // --- LEITURA DA MEMÓRIA ---
  preferences.begin("creds", true);
  ssid_cliente = preferences.getString("ssid", "");
  pass_cliente = preferences.getString("pass", "");
  apikey_cliente = preferences.getString("apikey", "");
  url_cliente = preferences.getString("url", "");
  preferences.end();

  if (ssid_cliente == "" || apikey_cliente == "") {
    Serial.println("[ERRO] Credenciais ausentes. Volte ao Estagio 1.");
    while (true) delay(1000);
  }

  // --- CONEXÃO WI-FI ---
  Serial.print("[WIFI] Conectando a: ");
  Serial.println(ssid_cliente);
  WiFi.begin(ssid_cliente.c_str(), pass_cliente.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\\n[WIFI] Conectado! IP: " + WiFi.localIP().toString());

  // --- INICIALIZAÇÃO FIREBASE (Mobizt) ---
  config.api_key = apikey_cliente.c_str();
  config.database_url = url_cliente.c_str();
  
  // Usamos login anónimo simples para leitura/escrita rápida em bases públicas
  auth.user.email = "";
  auth.user.password = "";

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Regista as informações estáticas iniciais no Firebase
  Firebase.RTDB.setString(&fbdo, "chacara/dispositivos/" + DISPOSITIVO_ID + "/modelo_hardware", MODELO_HARDWARE);
  Firebase.RTDB.setString(&fbdo, "chacara/dispositivos/" + DISPOSITIVO_ID + "/versao_atual", VERSAO_ATUAL);
  
  Serial.println("[FIREBASE] Configurado e Online.");
}

void loop() {
  // Executa de 10 em 10 segundos
  if (Firebase.ready() && (millis() - tempoAnterior > intervaloReporte || tempoAnterior == 0)) {
    tempoAnterior = millis();
    
    // 1. Reportar Uptime (Avisa o HTML que a placa está viva)
    int uptime = millis() / 1000;
    Firebase.RTDB.setInt(&fbdo, "chacara/dispositivos/" + DISPOSITIVO_ID + "/status_uptime", uptime);

    // 2. Verificar se há ordem de Atualização (OTA)
    if (Firebase.RTDB.getBool(&fbdo, "chacara/dispositivos/" + DISPOSITIVO_ID + "/comando_ota")) {
      bool otaPendente = fbdo.boolData();
      
      if (otaPendente) {
        Serial.println("\\n[OTA] ALERTA: Comando de atualizacao recebido!");
        
        // Desligar flag no Firebase para não entrar em loop infinito
        Firebase.RTDB.setBool(&fbdo, "chacara/dispositivos/" + DISPOSITIVO_ID + "/comando_ota", false);
        
        // Ir buscar a URL do firmware ao Firebase
        if (Firebase.RTDB.getString(&fbdo, "config/arquivos_ota/" + MODELO_HARDWARE)) {
          String urlArquivo = fbdo.stringData();
          Serial.println("[OTA] URL recebida: " + urlArquivo);
          
          // Iniciar Processo de Download e Flash
          executarOTA(urlArquivo);
        }
      }
    }
  }
}

// =================================================================================
// FUNÇÃO DE DOWNLOAD E ATUALIZAÇÃO (HTTPUpdate)
// =================================================================================
void executarOTA(String urlDownload) {
  Serial.println("[OTA] Iniciando conexao segura com o GitHub...");
  
  WiFiClientSecure client;
  // O GitHub usa HTTPS. setInsecure() permite baixar sem precisar do certificado SSL raiz (mais simples e direto).
  client.setInsecure(); 
  
  // O GitHub faz redirecionamentos automáticos de links. Esta linha é obrigatória.
  httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  
  Serial.println("[OTA] Baixando e instalando... Aguarde.");
  
  // A execução do OTA congela o resto do código até terminar
  t_httpUpdate_return ret = httpUpdate.update(client, urlDownload);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("[OTA ERRO] Falha na Atualizacao (%d): %s\\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("[OTA INFO] Nenhuma atualizacao disponível.");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("[OTA SUCESSO] Atualizacao concluida. Reiniciando...");
      // O ESP32 reinicia automaticamente após o SUCESSO
      break;
  }
}
"""
with open("Exemplo_Final_OTA_Firebase.ino", "w", encoding="utf-8") as f:
    f.write(cpp_content)
print("Your code file is ready\n[file-tag: Exemplo_Final_OTA_Firebase.ino]")
