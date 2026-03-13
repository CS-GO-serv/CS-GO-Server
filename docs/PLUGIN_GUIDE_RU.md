# PLUGIN GUIDE (RU): ServerFlow

## 1. Что сейчас актуально

Актуальный плагин: **ServerFlow**.

- Точка входа компиляции: `csgo/addons/sourcemod/scripting/serverflow.sp`
- Корневой orchestration-файл: `csgo/addons/sourcemod/scripting/serverflow/serverflow.sp`
- Целевой бинарник: `csgo/addons/sourcemod/plugins/serverflow.smx`

Legacy ModeVote сохранён только как архив исходников:
`csgo/addons/sourcemod/scripting/serverflow/archive/mode_vote.legacy.sp`

## 2. Сборка и запуск

### Сборка
```bat
build_serverflow.bat
```

### Legacy shim
```bat
build_mode_vote.bat
```
(проксирует вызов в `build_serverflow.bat`)

### Старт
- `start.bat`
- `start_classic.bat`
- `start_dz.bat`

Все стартовые скрипты используют `build_serverflow.bat`.

## 3. Конфиги

ServerFlow-конфиги находятся в `csgo/addons/sourcemod/configs/serverflow/`:
- `plugin_core.cfg`
- `scenarios.cfg`
- `playlists.cfg`
- `lang_ru.cfg`
- `lang_en.cfg`
- `examples/scenarios.example.cfg`
- `examples/playlists.example.cfg`

## 4. Архитектурный источник истины

Сверка архитектуры и требований:
1. `docs/design_doc_csgo_server_plugin_ru.md`
2. `docs/technical_spec_codex_csgo_plugin_ru.md`
3. `docs/implementation_roadmap_file_structure_codex_csgo_plugin_ru.md`
4. `docs/agents_md_codex_csgo_plugin_ru.md`

## 5. Статус миграции

ServerFlow уже имеет модульную структуру и базовые подсистемы.
Но часть поведения всё ещё доводится до полного соответствия спецификации (state machine/transition/pending-confirm/apply/UX-polish/diagnostics).
