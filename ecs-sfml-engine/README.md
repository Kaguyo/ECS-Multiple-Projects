# ECS Animation Engine

Ferramenta de autoria para **animações e movesets** baseados em ECS (Entity-Component-System), com geração opcional de código **TypeScript** e **C++** a partir de definições visuais feitas na engine.

---

## Propósito

A engine permite definir, de forma visual e interativa, como inputs geram eventos — e a partir disso gera automaticamente as implementações correspondentes em projetos externos. O foco é criar fluxos de animação e moveset com **previsibilidade de simulação**, através de abstrações que eliminam boilerplate e reduzem erros de integração.

A saída é flexível: o mesmo moveset definido na engine pode exportar para uma arquitetura orientada a eventos em C++ ou TypeScript, adaptando-se à estrutura do projeto alvo.

---

## Como funciona

### 1. Aponte seu projeto

Você informa à engine dois caminhos dentro do seu projeto existente:

```
ECS/    ← pasta de entidades, componentes e sistemas
Core/   ← pasta com Input/ e Events/ do projeto
```

A engine usa essas pastas para **testes de compatibilidade de integração** — ela analisa o que já existe antes de gerar qualquer coisa.

### 2. Defina inputs e eventos na engine

Dentro da interface da engine, você associa qualquer input a um evento:

```
[ INPUT: KeyPress("Attack") ]  →  [ EVENT: OnAttackTriggered ]
[ INPUT: KeyPress("Dodge")  ]  →  [ EVENT: OnDodgeStarted    ]
```

Antes de gerar código, a engine **valida** se o evento já existe em `Core/Events/`. Se existir, reutiliza. Se não existir, **gera automaticamente** a nova implementação que entregue o processamento definido.

### 3. O que a engine modifica

```
projeto-alvo/
├── ECS/          ← engine modifica: componentes, entidades, sistemas de moveset
└── Core/
    ├── Input/    ← engine modifica: mapeamentos de input
    └── Events/   ← engine valida e gera: definições e handlers de eventos
```

> Apenas essas pastas são tocadas. O restante do projeto permanece intacto.

---

## Fluxo de integração

```
Engine (definição visual)
        │
        ▼
  Análise de Core/Events/
  "OnAttackTriggered já existe?"
        │
   ┌────┴────┐
  SIM       NÃO
   │         │
   │         ▼
   │    Gera implementação
   │    em C++ ou TypeScript
   │         │
   └────┬────┘
        ▼
  Atualiza ECS/
  (componentes e sistemas do moveset)
        │
        ▼
  Relatório no console da engine
  com o que foi criado / reutilizado
```

---

## Stack

| Camada | Tecnologia |
|---|---|
| Engine (interface) | C++17 + SFML 3 |
| Geração de código alvo | C++ ou TypeScript |
| Arquitetura alvo | Event-driven + ECS |
| Build da engine | MinGW WinLibs GCC 14.2 / UCRT |

---

## Documentação técnica

Ver [`DOCS.md`](./DOCS.md) para arquitetura interna da engine, módulos, threading model e regras SFML 3.
