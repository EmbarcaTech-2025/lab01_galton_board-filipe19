# **EmbarcaTech HBr 2025**  

 Institution: Instituto Hardware BR-DF  
 Course: **Technological Residency in Embedded Systems**  
 Author: **Filipe Alves de Sousa**  
 Brasília-DF, May 2025  
 
# **Digital Galton Board**  

This project implements a digital version of a Galton Board (or Plinko), demonstrating how a series of random binary decisions leads to a **normal probability distribution**.  

![Image](https://github.com/user-attachments/assets/7b051d18-c318-43bf-a24c-10185dbc472d)
_Foto do display oled durante os testes - developmente environment._  


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

![Image](https://github.com/user-attachments/assets/f7c703dd-2d34-49cb-bfd7-568ae25165a4)
_Foto da BitDogLab e ao fundo, o VSCode developmente environment._  

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


![Image](https://github.com/user-attachments/assets/01cdeb94-b9c7-456d-ac25-e762928edade)
_Photo of my implementation in the VSCode developmente environment._  

---

## *Controle interativo:* 
Botão A (GP5):

Cicla entre 1-5 bolas liberadas simultaneamente

Exibido como "A:X" no canto superior esquerdo

Botão B (GP6):

Ajusta o desbalanceamento (0-10)

0 = Viés máximo para esquerda

5 = Distribuição balanceada (padrão)

10 = Viés máximo para direita

Exibido como "B:X" no canto superior direito


## Exibição de Dados:
*Total de bolas:* "T:XXXX" no canto inferior esquerdo

*Histograma:*
Normalizado automaticamente
Altura máxima configurável (MAX_HISTOGRAM_HEIGHT)

*Elementos visuais:*
Pinos hexagonais
Trajetórias das bolas
Canaletas / receptáculos


## Princípios de Funcionamento

**Física da Simulação**
```c
void update_particles() {
    // Aplica gravidade
    particles[i].vy += GRAVITY;
    
    // Atualiza posição
    particles[i].x += particles[i].vx;
    particles[i].y += particles[i].vy;

    // Colisões elásticas com coeficiente BOUNCINESS
    if (collision) {
        particles[i].vx = -particles[i].vx * BOUNCINESS;
    }
}
```

Algoritmo de Decisão com Viés
```c
bool random_decision_with_bias() {
    int threshold = map(BALANCE_BIAS, 0, 10, 5, 95);
    return (rand() % 100) < threshold;
}
```

Normalização do Histograma
```c
void normalize_histogram() {
    uint16_t max_val = find_max_value();
    for (int i = 0; i < NUM_BINS; i++) {
        histogram[i] = (histogram[i] * MAX_HISTOGRAM_HEIGHT) / max_val;
    }
}
```

## Parâmetros Ajustáveis:
- Parâmetro	Valores	Efeito
- GRAVITY	0.1-0.5	Ajusta velocidade de queda
- BOUNCINESS	0.1-0.9	Controla elasticidade nas colisões
- PIN_SPACING_HORIZONTAL	5-15	Espaçamento entre pinos
- MAX_HISTOGRAM_HEIGHT	10-20	Altura máxima do gráfico

## **Teoria Matemática Implementada:**
Distribuição Binomial: Simulada pelo padrão de colisões
## **Lei dos Grandes Números:** Demonstrada pela convergência do histograma
Processos Estocásticos: Decisões aleatórias com viés controlável

## Como Compilar e Executar
Clone o repositório
Configure o ambiente de desenvolvimento para Raspberry Pi Pico
Conecte os componentes conforme o diagrama
Compile e carregue o firmware:

```bash
mkdir build && cd build
cmake ..
make
```


## Exemplos de Funcionamento

### 1. Contador Total de Bolas

Abaixo, o contador no canto da tela aumenta conforme novas bolas são geradas e caem nos bins:


![Contador Total de Bolas](https://github.com/user-attachments/assets/89c5a211-af18-403d-8df4-4433f3f387c8)

_Photo of my implementation - Contador Total de Bolinhas._  


---

### 2. Simulação com Desbalanceamento

Aqui, o botão foi usado para aplicar viés à direita. Note a assimetria da distribuição final:

![Simulação com Viés](https://github.com/user-attachments/assets/5a47eb1e-7c77-463e-b45d-708d8b0dd126)


_Photo of my implementation - Simulação com Viés - tendência para esquerda/direita._  

---

### 3. Múltiplas Bolas Simultâneas

Nesta simulação, várias partículas caem ao mesmo tempo, aumentando a velocidade da formação do histograma:

![Várias Bolas ao Mesmo Tempo](https://github.com/user-attachments/assets/c9208a5b-c449-4274-82e7-0fdb2ce7744b)

_Photo of my implementation - Várias Bolas ao Mesmo Tempo._  



## Demonstração em Vídeo 

[![Assista ao vídeo da Placa de Galton Digital](https://img.youtube.com/vi/FgTVdCW7lLk/0.jpg)](https://www.youtube.com/watch?v=FgTVdCW7lLk)

_Assista ao vídeo da Placa de Galton Digital._  

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
* Botões
* Ambiente de desenvolvimento para C/C++ com SDK da Pico

## Referências
- "The Galton Board: 150 Years of Normal Distribution" - Statistical Science
- Raspberry Pi Pico C/C++ SDK Documentation
- Principios de Simulação Física - David H. Eberly

## **License**

This project is licensed under the [MIT License](LICENSE).

 