/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Ian Craggs - make sure QoS2 processing works, and add device headers
 *******************************************************************************/

 /**
  This is a sample program to illustrate the use of the MQTT Client library
  on the mbed platform.  The Client class requires two classes which mediate
  access to system interfaces for networking and timing.  As long as these two
  classes provide the required public programming interfaces, it does not matter
  what facilities they use underneath. In this program, they use the mbed
  system libraries.

 */

 // change this to 1 to output messages to LCD instead of serial
 
#define USE_LCD 0

#if USE_LCD
#include "C12832.h"

// the actual pins are defined in mbed_app.json and can be overridden per target
C12832 lcd(LCD_MOSI, LCD_SCK, LCD_MISO, LCD_A0, LCD_NCS);

#define logMessage lcd.cls();lcd.printf

#else

#define logMessage printf // Criação do Log de mensagens

#endif

#define MQTTCLIENT_QOS2 1

// Inclusão das biliotecas necessárias para comunicação via Mosquito

    #include "easy-connect.h"
    #include "MQTTNetwork.h"
    #include "MQTTmbed.h"
    #include "MQTTClient.h"
    #include <string>
    #include "TextLCD.h"

// Definição da Serial via USB

    Serial MotorPLC(USBTX, USBRX); 
    Serial Teste(PD_5, PD_6);

// Definição do I2C

    I2C OLED(D14, D15);

    TextLCD_I2C lcd(&OLED, 0x4E, TextLCD::LCD20x4); 

// Contagem de mensagens recebidas

    int arrivedcount = 0; 

// Criação de Buffer de armazenamento de mensagem

    char buf[2];
    char buf2[10];

// Armazenamento de letra

    char caracter;

// Criação de Threads

    Thread serial;
    Thread tela;

// Função de identificação de mensagem recebida via Mosquitos

    void messageArrived(MQTT::MessageData& md) {

    // Armazenamento da mensagem e informações referentes à mesma 

            MQTT::Message &message = md.message;

        // Aumento da contagem de mensagens recebidas pela placa

            ++arrivedcount;

    }

// Função para adquirir caracter vindo da Serial 

    void caracteres() {

        // Loop de funcionamento da função

            while(1) {

                // Recebimento de caracter via Serial e alocação do dado para o buffer

                    caracter = Teste.getc();

                    sprintf(buf2, "%c", caracter);

            }

    }

// Função para impressão de texto no LCD

    void texto() {

        // Loop de funcionamento da função

            while(1) {

                // Caso o caracter recebido seja M, é indicado no LCD que o motor está habilitado em sentido horário

                    if (caracter == 'M') {
                    
                        lcd.locate(0, 0);
                        lcd.printf("Habilitado  ");
                        lcd.locate(0, 1);
                        lcd.printf("Horario    ");
                        lcd.setCursor(TextLCD::CurOff_BlkOff);
                        wait(1);

                    }
                
                // Caso o caracter recebido seja W, é indicado no LCD que o motor está habilitado em sentido antihorário

                    else if (caracter == 'W') {

                        lcd.locate(0, 0);
                        lcd.printf("Habilitado  ");
                        lcd.locate(0, 1);
                        lcd.printf("AntiHorario");
                        lcd.setCursor(TextLCD::CurOff_BlkOff);
                        wait(1);

                    }

                // Caso o caracter recebido seja D, é indicado no LCD que o motor está desabilitado

                    else if (caracter == 'D') {

                        lcd.locate(0, 0);
                        lcd.printf("Desabilitado");
                        lcd.locate(0, 1);
                        lcd.printf("-----------");
                        lcd.setCursor(TextLCD::CurOff_BlkOff);
                        wait(1);

                    }

            }

    }

// Função principal de funcionamento

    int main(int argc, char* argv[]) {

        // Indicação de começo de operação via LCD

            lcd.locate(0, 0);
            lcd.printf("Inicio de");
            lcd.locate(0, 1);
            lcd.printf("Conexao");
            wait(2);

        // Limpeza da tela

            lcd.cls();

        // Definição das velocidades de resposta da Serial

            MotorPLC.baud(115200);
            Teste.baud(115200);

        // Definição da versão de Mosquito e do tópico de comunicação com o seervidor Mosquito

            float version = 0.6;
            char* topic = "automation/topic";

        // Mensagem de Log 

            logMessage("HelloMQTT: version is %.2f\r\n", version);

        // Definição da rede 

            NetworkInterface* network = easy_connect(true);
        
        // Caso a conexão falhe, haverá um retorno para sinalização do erro

            if (!network) {

                lcd.locate(0, 0);
                lcd.printf("Erro");

                return -1;
                
            }

        // Conexão com a rede Mosquito

            MQTTNetwork mqttNetwork(network);

        // Definição do cliente conectado à rede Mosquito previamente definida

            MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

        // Definição do host da rede e da porta de acesso

            const char* hostname = "192.168.50.179";
            int port = 1883;
        
        // Mensagem sinalizando tentativa de conexão com o host 

            logMessage("Connecting to %s:%d\r\n", hostname, port);

        // Flag de conexão realizada com o host

            int rc = mqttNetwork.connect(hostname, port);

        // Se a flag for diferente de 0, uma mensagem sinalizando a conexão bem sucedida é mostrada

        if ((rc = mqttNetwork.connect(hostname, port)) != 0)
            logMessage("rc from TCP connect is %d\r\n", rc);

        // Criação do usuário conectado à rede

            MQTTPacket_connectData data = MQTTPacket_connectData_initializer; // Inicialização de dados
            data.MQTTVersion = 3; // Versão de MQTT
            data.clientID.cstring = "XNUCLEOPLC01A1"; // ID do Cliente
            data.username.cstring = "MotorPLC"; // Nome de usuário
            data.password.cstring = "ProjetoAutomacao"; // Senha necessária de acesso

        // Caso o cliente seja detectado pelo host, uma mensagem é exibida 

            if ((rc = client.connect(data)) != 0)
                logMessage("rc from MQTT connect is %d\r\n", rc);

        // Caso o cliente venha a se inscrever em certo tópico da rede, uma mensagem é exibida 

            if ((rc = client.subscribe(topic, MQTT::QOS0, messageArrived)) != 0)
                logMessage("rc from MQTT subscribe is %d\r\n", rc);

        // Variável de criação da mensagem a ser enviada pelo canal no tópico conectado

            MQTT::Message message;    

        // Inicialização da Thread

            serial.start(&caracteres);
            tela.start(&texto);

        // Enquanto o cliente estiver conectado na rede, ele fica enviando informações no canal relacionado ao tópico desejado 
            
            while(1) {

                // Criação da mensagem a ser enviada através da atribuição do buffer para a mensagem

                    message.qos = MQTT::QOS2; // Sinalização da Mensagem
                    message.retained = false;
                    message.dup = false; 
                    message.payload = (void*)buf2; // Atribuição do buffer
                    message.payloadlen = strlen(buf2)+1; // Tamanho da mensagem
                    client.publish(topic, message); // Publicação da mensagem no tópico conectado

            }

    }
