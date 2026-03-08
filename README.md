# video-compressor-and-terminal-video-player

Este projeto é um experimento simples de **compressão e reprodução de vídeo em 1 bit por pixel**.
O projeto contém duas ferramentas principais:

* **Conversor** – transforma um fluxo de pixels RGB em um vídeo binário.
* **Player** – reproduz o vídeo no terminal usando `ncurses`.

A ideia é reduzir drasticamente o tamanho do vídeo representando cada pixel apenas como **preto ou branco**.

---

# Formato do arquivo

Os arquivos gerados usam a extensão `.bit`.

Estrutura do arquivo:

```
[ metadata ][ video data ]
```

## Metadata (4 bytes)

| Campo     | Tipo | Descrição                |
| --------- | ---- | ------------------------ |
| fps       | u8   | Frames por segundo       |
| width     | u8   | Largura do vídeo         |
| height    | u8   | Altura do vídeo          |
| grayscale | u8   | Reservado (atualmente 0) |

## Video Data

Depois do header, o vídeo é armazenado como **bits sequenciais**.

Cada bit representa **1 pixel**:

```
1 = pixel branco
0 = pixel preto
```

Os pixels são armazenados em ordem:

```
linha por linha
frame por frame
```

Exemplo conceitual de um frame 8x2:

```
11110000
00001111
```

Isso ocupa apenas:

```
16 bits = 2 bytes
```

---

# Conversão de vídeo

O conversor recebe um arquivo contendo pixels RGB e converte cada pixel para **preto ou branco**.

Para cada pixel:

```
grayscale = (R + G + B) / 3
```

Depois é aplicado um threshold:

```
se grayscale >= 128 → pixel branco
se grayscale < 128  → pixel preto
```

O resultado é armazenado como **um único bit**.

---

# Compilação

Compile os programas com `gcc`.

Conversor:

```
gcc converter.c -o converter
```

Player:

```
gcc player.c -lncurses -o player
```

---

Você pode adicionar uma seção curta no README explicando o uso do **ffmpeg** para gerar o arquivo `.raw` que seu conversor espera.

---

## Gerando o arquivo RAW com ffmpeg

Antes de usar o conversor, é necessário transformar um vídeo comum em um fluxo de pixels **RGB bruto (raw)** com resolução reduzida. Isso pode ser feito usando `ffmpeg`.

Exemplo:

```
ffmpeg -i input.mp4 -vf scale=80:40 -pix_fmt rgb24 -f rawvideo video.raw
```

Parâmetros principais:

| Opção             | Descrição                               |
| ----------------- | --------------------------------------- |
| `-i input.mp4`    | vídeo de entrada                        |
| `-vf scale=80:40` | reduz a resolução do vídeo              |
| `-pix_fmt rgb24`  | força o formato RGB (3 bytes por pixel) |
| `-f rawvideo`     | gera saída sem container                |
| `video.rgb`       | arquivo de saída                        |

O arquivo gerado contém apenas os pixels em sequência:

```
R G B R G B R G B ...
```

Cada pixel ocupa **3 bytes**.

Esse arquivo pode então ser passado para o conversor do projeto para gerar o formato `.bit`.


# Uso

## Converter vídeo

```
./converter input.raw height width fps
```

Exemplo:

```
./converter video.raw 40 80 30
```

Isso gera:

```
video.bit
```

---

## Reproduzir vídeo

```
./player video.bit
```

O player usa `ncurses` para desenhar cada frame no terminal.

Cada pixel é exibido como três caracteres para melhorar a proporção:

```
@@@
```

Pixels pretos são espaços.

---

# Como o player funciona

1. O arquivo `.bit` é carregado na memória.
2. O header é lido para obter:

   * largura
   * altura
   * fps
3. O número total de frames é calculado.
4. Cada frame é renderizado no terminal.
5. O tempo entre frames é controlado usando `nanosleep`.

---

# Objetivo do projeto

Este projeto demonstra:

* armazenamento extremamente compacto de vídeo
* manipulação de bits em C
* renderização de vídeo em terminal
* criação de um formato de mídia simples

É um experimento minimalista para entender **compressão e reprodução de vídeo em baixo nível**.
