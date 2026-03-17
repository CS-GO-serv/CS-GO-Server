# CS-GO-Server

Репозиторий содержит конфиги сервера CS:GO Legacy и SourceMod-плагин **ServerFlow** (новая state-driven архитектура).

## Официальная точка входа плагина

- Источник: `csgo/addons/sourcemod/scripting/serverflow.sp`
- Корневой модуль: `csgo/addons/sourcemod/scripting/serverflow/serverflow.sp`
- Выходной бинарник: `csgo/addons/sourcemod/plugins/serverflow.smx`

`mode_vote` больше не является основной архитектурой и не участвует в runtime/build path. Старый код сохранён только как историческая справка в архиве (**not runtime path**):
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
Legacy build-chain удалён из активного run-flow. Единственный runtime/build entrypoint — **ServerFlow** через `build_serverflow.bat`; архивный `mode_vote` — только historical reference (**not runtime path**).

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
- `docs/human_centered_ux_guidelines_for_codex_csgo_plugin_ru.md`

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

### Термины map pool (синхронизировано с `docs/PLUGIN_GUIDE_RU.md`)
- `playlist_id` — ссылка из сценария (`scenarios.cfg`) на секцию в `playlists.cfg`.
- `maplist_policy` — кто source of truth для map pool:
  - `playlist` -> map pool берётся из `playlists.cfg` (`maplist_file` плейлиста по `playlist_id`).
  - `scenario` -> map pool берётся из `scenarios.cfg` (`maplist_file` сценария).

Примеры:
- `maplist_policy=playlist`: `playlist_id="comp"`, карта-пул читается из `playlists.cfg` -> `"comp"` -> `maplist_file`.
- `maplist_policy=scenario` (поддерживается): сценарий задаёт собственный `maplist_file`, и он становится source of truth.

> Важно: ServerFlow — единственная поддерживаемая архитектура в активном run/build-пайплайне. Документы в `docs/` являются source of truth.


## Scope PR

Разрешённый scope для активных PR ограничен whitelist-областями:
- `csgo/addons/sourcemod/scripting/serverflow/**` и `csgo/addons/sourcemod/scripting/serverflow.sp`
- `csgo/addons/sourcemod/configs/serverflow/**`
- `docs/**`
- старт/билд скрипты: `build_serverflow.bat`, `start.bat`, `start_classic.bat`, `start_dz.bat`
- нужные текстовые артефакты: `Log/**`, `serverflow.smx.b64.txt`

Из PR-диффа нужно исключать массовые изменения вне scope, в том числе:
- `csgo/addons/sourcemod/scripting/base*`
- `csgo/addons/sourcemod/scripting/fun*`
- `csgo/addons/sourcemod/scripting/include/*`
- `csgo/addons/sourcemod/scripting/testsuite/*`

Если такие изменения попали в рабочую ветку случайно, удаляйте их из диффа перед review, например:

```bash
git restore --staged --worktree \
  csgo/addons/sourcemod/scripting/base* \
  csgo/addons/sourcemod/scripting/fun* \
  csgo/addons/sourcemod/scripting/include \
  csgo/addons/sourcemod/scripting/testsuite
```
