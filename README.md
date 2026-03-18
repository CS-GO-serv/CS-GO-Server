# CS-GO-Server

Репозиторий содержит конфиги сервера CS:GO Legacy и SourceMod-плагин **ServerFlow** (новая state-driven архитектура).

## Официальная точка входа плагина

- Источник: `csgo/addons/sourcemod/scripting/serverflow.sp`
- Корневой модуль: `csgo/addons/sourcemod/scripting/serverflow/serverflow.sp`
- Выходной бинарник: `csgo/addons/sourcemod/plugins/serverflow.smx`

`mode_vote` больше не является основной архитектурой. Старый код сохранён только в архиве:
`csgo/addons/sourcemod/scripting/serverflow/archive/mode_vote.legacy.sp`.

## Build / run

### Сборка (канонично)
```bat
build_serverflow.bat
```

### Важно для PR/Review
- Бинарные артефакты (`*.smx`, `spcomp*.exe`, `compile.exe/dat`) не должны попадать в PR-дифф.
- Для переносимого хранения артефактов используйте только текстовые Base64-файлы:
  - `csgo/addons/sourcemod/plugins/serverflow.smx.b64.txt`
  - `csgo/addons/sourcemod/scripting/compiled/serverflow.smx.b64.txt`

### Legacy-совместимость
Legacy build-chain удалён из активного run-flow. Официальная сборка только через `build_serverflow.bat`.

### Старт сервера
- `start.bat` (канонично)
- `start_classic.bat` (deprecated shim -> `start.bat`)
- `start_dz.bat` (deprecated shim -> `start.bat`)

`start.bat` — единая точка входа для запуска сервера. Дополнительные bat-файлы оставлены только как совместимые shim-обёртки для старых процессов запуска.

## Архитектура

Реализация разбита по слоям в `csgo/addons/sourcemod/scripting/serverflow/`:
- `core/`
- `runtime/`
- `config/`
- `domain/`
- `presentation/`
- `commands/`
- `diagnostics/`
- `integrations/`

Структура и целевая модель описаны в:
- `docs/design_doc_csgo_server_plugin_ru.md`
- `docs/technical_spec_codex_csgo_plugin_ru.md`
- `docs/implementation_roadmap_file_structure_codex_csgo_plugin_ru.md`
- `docs/agents_md_codex_csgo_plugin_ru.md`
- `docs/human_centered_ux_guidelines_for_codex_csgo_plugin_ru.md`
- `docs/additional_functionality_ru.md`

## Текущий статус реализации (continuation)

- ✅ Каноничный pipeline: `build_serverflow.bat` -> `serverflow.smx`.
- ✅ State-driven runtime с pending/confirm/apply и TransitionManager как точкой применения.
- ✅ Команды для игроков/админов и расширенный статус (`sm_status_verbose`).
- ⚠️ Дальнейшая задача roadmap: углубление UX-флоу и полировка runtime-policy в edge-cases.

## Быстрый smoke-check после обновления

1. `sm plugins list` — убедиться, что `ServerFlow` loaded без startup errors.
2. `sm_status` и `sm_status_verbose` — проверить читаемые текущий state и next action.
3. `sm_queuescenario dz` -> `sm_confirmpending` — проверить controlled apply через pending pipeline.
4. `sm_team` / `sm_ready` / `sm_unready` — проверить PreMatch UX и реакцию статуса.

## Конфиги ServerFlow

- `csgo/addons/sourcemod/configs/serverflow/plugin_core.cfg`
- `csgo/addons/sourcemod/configs/serverflow/scenarios.cfg`
- `csgo/addons/sourcemod/configs/serverflow/playlists.cfg`
- `csgo/addons/sourcemod/configs/serverflow/lang_ru.cfg`
- `csgo/addons/sourcemod/configs/serverflow/lang_en.cfg`
- `csgo/addons/sourcemod/configs/serverflow/examples/*`

> Важно: ServerFlow — единственная поддерживаемая архитектура в активном run/build-пайплайне. Документы в `docs/` являются source of truth.


## Репозиторный cleanup (release discipline)

- В активном run-flow каноничным остаётся только `start.bat`.
- `start_classic.bat` и `start_dz.bat` сохранены как deprecated shim-обёртки, чтобы уменьшить риски миграции и конфликтов merge в старых ветках.
- Build-скрипт не должен модифицировать исходники на лету: проблемы источников исправляются только через git.
