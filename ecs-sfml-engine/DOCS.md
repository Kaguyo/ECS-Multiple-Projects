# ECS SFML Engine — Documentação v1.0
> SFML 3 · C++17 · MinGW (WinLibs GCC 14.2.0 UCRT)

---

## Índice
1. [Visão Geral](#1-visão-geral)
2. [Estrutura de Arquivos](#2-estrutura-de-arquivos)
3. [Arquitetura de Threads](#3-arquitetura-de-threads)
4. [Módulos](#4-módulos)
5. [Layout Visual da UI](#5-layout-visual-da-ui)
6. [Fluxo de Eventos](#6-fluxo-de-eventos)
7. [Paleta de Cores (Theme)](#7-paleta-de-cores-theme)
8. [Build (tasks.json)](#8-build-tasksjson)
9. [Regras Críticas SFML 3](#9-regras-críticas-sfml-3)

---

## 1. Visão Geral

Interface da **ECS Animation Engine** — ferramenta de autoria para animações e movesets baseados em ECS, com geração de código C++ e TypeScript para projetos externos.

A janela 1100×700 é dividida em três zonas:
- **Sidebar** — painel lateral com ações da engine (spawn, sistemas, integração)
- **Content Area** — viewport principal para simulação e preview de movesets
- **Console Bar** — log de operações, validações e relatórios de geração de código

O projeto separa **processamento de eventos** (main thread) de **renderização** (render thread), seguindo as restrições do SFML 3 para multithreading com OpenGL. Todo input do usuário na engine transita pelo `EventBus` — o mesmo padrão arquitetural usado na geração de código para projetos alvo.

---

## 2. Estrutura de Arquivos

```
ecs-sfml-engine/
├── main.cpp           ← ponto de entrada (3 linhas)
├── Application.hpp    ← orquestra tudo; contém o event loop
├── Renderer.hpp       ← thread de render (clear/draw/display)
├── UI.hpp             ← componentes visuais (Sidebar, Console, Botões)
├── SharedState.hpp    ← dados compartilhados entre threads com mutex
├── EventBus.hpp       ← pub/sub singleton desacoplado
└── .vscode/
    ├── tasks.json     ← compila com g++ + flags SFML 3 estático
    └── launch.json    ← debug com gdb (WinLibs)
```

**Dependências de include:**

```
main.cpp
  └── Application.hpp
        ├── Renderer.hpp
        │     ├── SharedState.hpp
        │     └── UI.hpp
        │           └── EventBus.hpp
        ├── SharedState.hpp
        └── EventBus.hpp
```

---

## 3. Arquitetura de Threads

```
┌─────────────────────────────────────────────────────────────┐
│  MAIN THREAD  (obrigatório pela OS / SFML 3)                │
│                                                             │
│  Application::run()                                         │
│    window.setActive(false)  ← libera contexto OpenGL        │
│    renderer.start()         ← lança render thread           │
│                                                             │
│    loop: window.pollEvent()                                 │
│      → MouseMoved     → sidebar.handleMouseMove()           │
│      → MousePressed   → sidebar.handleMousePress()          │
│      → MouseReleased  → sidebar.handleMouseRelease()        │
│      → KeyPressed     → handleKeyShortcut()                 │
│      → Closed         → state.running=false, close()        │
└─────────────────────────┬───────────────────────────────────┘
                          │  SharedState (mutex + atomic)
┌─────────────────────────▼───────────────────────────────────┐
│  RENDER THREAD                                              │
│                                                             │
│  Renderer::loop()                                           │
│    window.setActive(true)   ← adquire contexto OpenGL       │
│    setFramerateLimit(60)                                    │
│                                                             │
│    while (state.running):                                   │
│      window.clear()                                         │
│      draw contentArea, divider                              │
│      sidebar.draw()                                         │
│      consoleBar.draw( state.snapshotConsole() )             │
│      window.display()                                       │
│                                                             │
│    window.setActive(false)  ← devolve contexto ao fechar    │
└─────────────────────────────────────────────────────────────┘
```

**Regra de ouro:** `pollEvent` sempre na thread que criou a janela. `draw/display` sempre na thread que possui o contexto (`setActive(true)`).

---

## 4. Módulos

### 4.1 `EventBus.hpp`
Singleton pub/sub. Desacopla quem dispara eventos de quem os trata.

```cpp
// Subscribing
EventBus::instance().subscribe("button_clicked", [](const std::string& payload) {
    // payload = label do botão clicado
});

// Publishing (feito internamente por SidebarButton)
EventBus::instance().publish("button_clicked", "1. Spawn Entity");
```

| Método | Thread-safe | Descrição |
|---|---|---|
| `subscribe(topic, handler)` | ✅ | Registra um handler para um tópico |
| `publish(topic, payload)` | ✅ | Dispara todos os handlers do tópico |

> ⚠️ O mutex do EventBus bloqueia durante o `publish`, logo handlers devem ser rápidos. Não chamar `publish` dentro de um handler do mesmo tópico (deadlock).

---

### 4.2 `SharedState.hpp`
Struct que concentra todo dado trocado entre as duas threads.

```
SharedState
  ├── std::atomic<bool> running      ← lido por render thread, escrito por main
  ├── std::vector<string> consoleLines
  ├── std::mutex consoleMutex
  ├── pushConsole(msg)               ← chamado pelo main thread via EventBus
  └── snapshotConsole()              ← chamado pelo render thread a cada frame
```

O `snapshotConsole()` retorna uma **cópia** do vector, garantindo que o render thread nunca segure o mutex por mais de uma instrução.

---

### 4.3 `UI.hpp`

Contém três classes e o namespace `Theme`:

#### `SidebarButton`
```
Construção: label + posição + tamanho + fonte
Estado visual: NORMAL → HOVER → PRESSED (via cores do Theme)
Ao pressionar: publica "button_clicked" no EventBus
```

#### `Sidebar`
```
WIDTH = 180px (constante estática)
Título "ACTIONS" no topo
6 botões enumerados com layout automático:
  yStart=56, btnH=38, gap=10, margin=12
```

#### `ConsoleBar`
```
HEIGHT = 150px (constante estática)
Posição: ancora no bottom da janela, largura total
Histórico máximo: 20 linhas (gerenciado pelo SharedState)
Scroll automático: sempre mostra as últimas N linhas visíveis
Cores alternadas por linha (TEXT_PRIMARY / TEXT_DIM)
```

---

### 4.4 `Renderer.hpp`
Encapsula a render thread. Não expõe `loop()` publicamente.

```cpp
Renderer renderer(window, state, font);

window.setActive(false);  // ANTES de start()
renderer.start();         // lança std::thread

// ... event loop ...

renderer.join();          // aguarda encerramento limpo
```

Expõe `sidebar()` para que o `Application` possa delegar eventos de mouse sem quebrar o encapsulamento.

---

### 4.5 `Application.hpp`
Ponto central de wiring. Responsabilidades:

1. Carrega fonte (`consola.ttf` → fallback `arial.ttf`)
2. Conecta EventBus: `"button_clicked"` → `state.pushConsole()`
3. Chama `window.setActive(false)` + `renderer.start()`
4. Roda o event loop e traduz eventos SFML → chamadas de UI
5. Mapeia teclas `1–6` para os mesmos eventos dos botões

---

## 5. Layout Visual da UI

```
┌────────────────────────────────────────────────────────────┐  ▲
│ SIDEBAR (180px)  │         CONTENT AREA                    │  │
│──────────────────│                                         │  │
│ ACTIONS          │   (reservado para cena / viewport ECS)  │  │
│                  │                                         │  700px
│ [1. Spawn Entity]│                                         │  │
│ [2. Clear Scene ]│                                         │  │
│ [3. Toggle Debug]│                                         │  │
│ [4. Run System  ]│                                         │  │
│ [5. Load Asset  ]│                                         │  │
│ [6. Save State  ]│                                         │  │
│                  │                                         │  │
├──────────────────┴─────────────────────────────────────────┤  │
│ CONSOLE BAR (150px altura)                                 │  │
│ CONSOLE                                                    │  │
│ [click] 1. Spawn Entity                                    │  ▼
│ [click] 3. Toggle Debug                                    │
└────────────────────────────────────────────────────────────┘
◄──────────────────────── 1100px ───────────────────────────►
```

**Zonas de coordenadas:**

```
Sidebar:      x: 0–180,    y: 0–700
Content Area: x: 180–1100, y: 0–550
Console Bar:  x: 0–1100,   y: 550–700
Divider:      x: 179–180,  y: 0–700  (1px accent color)
```

---

## 6. Fluxo de Eventos

```
Usuário clica botão "2. Clear Scene"
         │
         ▼
[MAIN THREAD] sf::Event::MouseButtonPressed
         │
         ▼
Application::run() → sidebar.handleMousePress(mousePos)
         │
         ▼
SidebarButton::handleMousePress()
  → muda cor para BTN_PRESS
  → EventBus::publish("button_clicked", "2. Clear Scene")
         │
         ▼
[EVENTBUS - ainda na main thread]
  → handler registrado em Application
  → state.pushConsole("[click] 2. Clear Scene")
         │  (mutex lock/unlock rápido)
         ▼
[RENDER THREAD - próximo frame]
  → state.snapshotConsole()  (cópia do vector)
  → ConsoleBar::draw(snapshot)
  → texto aparece na tela
```

**Atalhos de teclado** seguem o mesmo caminho a partir de `handleKeyShortcut()`:

```
Tecla "3" pressionada
  → handleKeyShortcut(sf::Keyboard::Key::Num3)
  → EventBus::publish("button_clicked", "3. Toggle Debug")
  → mesmo fluxo acima
```

---

## 7. Paleta de Cores (Theme)

| Token | RGB | Uso |
|---|---|---|
| `BG` | 18, 18, 28 | Fundo da content area |
| `SIDEBAR_BG` | 28, 28, 42 | Fundo da sidebar |
| `CONSOLE_BG` | 12, 12, 20 | Fundo do console |
| `BTN_NORMAL` | 45, 45, 70 | Botão em repouso |
| `BTN_HOVER` | 70, 70, 110 | Botão com mouse sobre |
| `BTN_PRESS` | 100, 100, 160 | Botão pressionado |
| `TEXT_PRIMARY` | 220, 220, 255 | Texto principal |
| `TEXT_DIM` | 120, 120, 160 | Texto alternado console |
| `ACCENT` | 90, 140, 255 | Bordas, divisores, títulos |

Todos definidos como `inline const sf::Color` no namespace `Theme` — alteráveis em um único lugar.

---

## 8. Build (tasks.json)

**Compilador:** `C:\winlibs\mingw64\bin\g++.exe`
**SFML:** `C:\SFML\` (versão MinGW GCC 13.1 SEH 64-bit)

Flags essenciais:
```
-std=c++17          obrigatório para structured bindings e std::optional
-DSFML_STATIC       liga libs estáticas (.a)
-IC:\SFML\include   headers SFML
-LC:\SFML\lib       libs SFML
-static             embute runtime no exe (sem DLLs externas)
-static-libgcc
-static-libstdc++
```

Libs linkadas (ordem importa):
```
-lsfml-graphics-s-d  -lsfml-window-s-d  -lsfml-system-s-d
-lsfml-main-s-d  -lfreetype
-lopengl32  -lwinmm  -lgdi32
-ld3d11  -ldxgi  -ld3dcompiler   ← SFML 3 Direct3D backend
-lole32  -loleaut32  -luuid
-mwindows                        ← suprime console window no Windows
```

Output: `${workspaceFolder}\build\main.exe`

---

## 9. Regras Críticas SFML 3

| Regra | Motivo |
|---|---|
| `pollEvent` apenas na main thread | OS exige que eventos sejam processados na thread que criou a janela |
| `window.setActive(false)` antes de lançar render thread | OpenGL context só pode estar ativo em uma thread por vez |
| `window.setActive(true)` dentro da render thread | Adquire o contexto para draw/display |
| `sf::Text` exige fonte no construtor | SFML 3 removeu construtor padrão de Text |
| `sf::Font` carregada via construtor | `loadFromFile` foi removido no SFML 3 |
| `event->getIf<T>()` para acessar dados | API SFML 3 — substitui `event.type == ...` do SFML 2 |
| `event->is<T>()` para checar tipo | API SFML 3 — substitui `event.type == ...` do SFML 2 |
