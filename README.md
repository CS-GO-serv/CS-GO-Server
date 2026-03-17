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

### Legacy-совместимость
Legacy build-chain удалён из активного run-flow. Официальная сборка только через `build_serverflow.bat`.

### Старт сервера
- `start.bat`
- `start_classic.bat`
- `start_dz.bat`

Все стартовые скрипты теперь вызывают `build_serverflow.bat`.

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

## Конфиги ServerFlow

- `csgo/addons/sourcemod/configs/serverflow/plugin_core.cfg`
- `csgo/addons/sourcemod/configs/serverflow/scenarios.cfg`
- `csgo/addons/sourcemod/configs/serverflow/playlists.cfg`
- `csgo/addons/sourcemod/configs/serverflow/lang_ru.cfg`
- `csgo/addons/sourcemod/configs/serverflow/lang_en.cfg`
- `csgo/addons/sourcemod/configs/serverflow/examples/*`

> Важно: ServerFlow — единственная поддерживаемая архитектура в активном run/build-пайплайне. Документы в `docs/` являются source of truth.
