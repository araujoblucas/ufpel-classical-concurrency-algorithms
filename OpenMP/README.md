
# Yvyrupa Code

**Yvyrupa Code** is a repository that gathers various implementations of classic algorithms and problems in multiple programming languages, focusing on comparing **concurrent and parallel programming interfaces**. The goal is to explore the **efficiency, efficacy, and effectiveness** of the concurrency models and native constructs available in modern languages.

## Objectives

This repository serves as a practical foundation for:

- Empirical study of the expressiveness and overhead of concurrency abstractions.
- Comparisons between models based on threads, channels, processes, and asynchronous tasks.
- Assessment of code readability, actual parallelism, and development effort.
- Investigation of the adoption of OpenMP directives in open-source codebases.

## Contents

The repository is organized by language, including implementations in:

- **C++ (C++20/C++23)** — using `std::thread`, `std::async`, `std::jthread`, `std::mutex`, OpenMP, etc.
- **Rust** — using `std::thread`, `mpsc` channels, `tokio`, `rayon`, among others.
- **Go** — using goroutines and channels (`chan`, `select`, `sync`, `waitgroup`).
- **Elixir** — using `spawn`, `send/receive`, `Task`, `Agent`, `GenServer`.

Each implementation addresses a classic problem (e.g., producer/consumer, Fibonacci calculation, Game of Life) to highlight different aspects of the concurrent and parallel programming models supported by each language.

## Citation

If you find this repository useful for your research or teaching, please cite the following references:

```bibtex
@inproceedings{OpenMPEmpiricalStudySBLP2024,
  author    = {Cristian Quevedo and Simone Cavalheiro and Marcos Oliveira Jr. and André Du Bois and Gerson Cavalheiro},
  title     = {An Empirical Study of OpenMP Directive Usage in Open-Source Projects on GitHub},
  booktitle = {Anais do XXVI Simpósio em Sistemas Computacionais de Alto Desempenho},
  location  = {Bonito/MS},
  year      = {2025},
  publisher = {SBC},
  address   = {Porto Alegre, RS, Brasil},
  pages     = {xxx--xxx},
  issn      = {0000-0000},
  doi       = {xxxxx},
  url       = {https://somewhereovertherainbow}
}

@incollection{CursoJAI2025,
  author    = {Gerson Cavalheiro and Alexandro Baldassin and André R. Du Bois},
  title     = {Programação Multithread: Modelos e Abstrações em Linguagens Contemporâneas},
  booktitle = {XLIV Jornadas de Atualização em Informática (JAI 2025)},
  location  = {Maceió/AL},
  year      = {2025},
  publisher = {SBC},
  address   = {Porto Alegre, RS, Brasil},
  pages     = {xxx--xxx},
  issn      = {0000-0000},
  doi       = {xxxxx},
  url       = {https://somewhereovertherainbow}
}
```

## About the name

The name **Yvyrupa Code** refers to the Guarani concept of *Yvyrupa* — the world as a shared, borderless land — symbolizing the coexistence of multiple programming languages within a unified analytical and practical space.
