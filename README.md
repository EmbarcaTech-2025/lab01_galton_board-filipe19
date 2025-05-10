# **EmbarcaTech HBr 2025**  

 Institution: Instituto Hardware BR-DF  
 Course: **Technological Residency in Embedded Systems**  
 Author: **Filipe Alves de Sousa**  
 Brasília-DF, May 2025  
 
# **Digital Galton Board**  

This project implements a digital version of a Galton Board (or Plinko), demonstrating how a series of random binary decisions leads to a **normal probability distribution**.  

## **Objective**  

Create an interactive visualization that simulates the behavior of a Galton Board using:  

## **Materials List**

| Component            | Connection on BitDogLab     |
|----------------------|-----------------------------|
| BitDogLab (RP2040)   | Raspberry Pi Pico W         |
| OLED Display I2C     | SDA: GPIO14 / SCL: GPIO15   |
| Button A             | GPIO5                       |
| Button B             | GPIO6                       |

---



# Digital Galton Board

Este projeto implementa uma versão digital da **Galton Board** utilizando a Raspberry Pi Pico com o kit **BitDogLab**. A simulação permite visualizar em tempo real o comportamento estatístico de partículas que colidem com pinos fixos e se acumulam em "bins", ilustrando conceitos como **distribuição binomial**, **Teorema Central do Limite** e a **Lei dos Grandes Números**.

---

## Funcionalidades

* **Simulação Visual em OLED**
  Um display OLED 128x64 mostra em tempo real:

  * As bolas caindo pela estrutura de pinos.
  * As colisões com física simplificada (gravidade e elasticidade).
  * O histograma final com o número de bolas acumuladas por bin.

* **Controle por botões**
  Os botões permitem ajustar:

  * botão [A] A quantidade de bolas geradas por ciclo.
  * botão [B] O grau de viés (desbalanceamento) para a direita ou esquerda.
 
* **Contador Total de Bolas [T]**
  Mostra no canto do display o número total de bolas lançadas durante a simulação.

* **Simulação com Várias Bolas Simultaneamente**
  Permite observar como múltiplas partículas interagem com o campo de pinos ao mesmo tempo, gerando padrões estatísticos mais rapidamente.

* **Desbalanceamento (Viés)**
  Uma função de viés ajustável altera a probabilidade de desvio da bola para esquerda ou direita ao colidir com um pino, possibilitando simular distribuições assimétricas.

---

## Organização do Projeto

```bash
digital-galton-board/
├── inc/
│   └── ssd1306.h           # Cabeçalhos da biblioteca para controle do display OLED
│   └── font8x8_basic.h     # Fonte 8x8 usada na renderização do display
│   └── ...                 # Outros arquivos auxiliares do display
├── src/
│   └── digital_galton_board.c # Código principal em C com toda a lógica da simulação
├── README.md               # Este arquivo de documentação
├── Makefile                # Script de compilação
└── assets/
    ├── gif_total_counter.gif       # GIF: Contador total de partículas
    ├── gif_desbalanceamento.gif    # GIF: Simulação com viés/desbalanceamento
    └── gif_multibola.gif           # GIF: Múltiplas bolas caindo simultaneamente
```

---

## Exemplos de Funcionamento

### 1. Contador Total de Bolas

Abaixo, o contador no canto da tela aumenta conforme novas bolas são geradas e caem nos bins:

![Contador Total de Bolas](assets/gif_total_counter.gif)

---

### 2. Simulação com Desbalanceamento

Aqui, o joystick foi usado para aplicar viés à direita. Note a assimetria da distribuição final:

![Simulação com Viés](assets/gif_desbalanceamento.gif)

---

### 3. Múltiplas Bolas Simultâneas

Nesta simulação, várias partículas caem ao mesmo tempo, aumentando a velocidade da formação do histograma:

![Várias Bolas ao Mesmo Tempo](assets/gif_multibola.gif)

---

## Como Compilar

Este projeto utiliza o **CMake** com o **SDK da Raspberry Pi Pico**. Para compilar:

```bash
git clone https://github.com/seuusuario/digital-galton-board.git
cd digital-galton-board
mkdir build && cd build
cmake ..
make
```

Depois, carregue o `.uf2` gerado para a sua Raspberry Pi Pico.

---

## Conceitos Demonstrados

* Distribuição Binomial
* Distribuição Normal (com grande número de partículas)
* Teorema Central do Limite
* Física simplificada de colisões (gravidade, elasticidade)
* Representação estatística em tempo real

---

## Requisitos

* Raspberry Pi Pico com BitDogLab ou similar
* Display OLED SSD1306 (via I2C)
* Joystick analógico com botões
* Ambiente de desenvolvimento para C/C++ com SDK da Pico


## **License**

This project is licensed under the [MIT License](LICENSE).

 