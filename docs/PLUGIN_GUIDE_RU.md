# PLUGIN GUIDE (RU): ServerFlow

> Этот документ — практический runbook для админов и тестеров.  
> Цель: чтобы можно было без путаницы собрать, запустить, проверить и диагностировать текущий ServerFlow.

---

## 1) Что это за плагин и текущий статус

`ServerFlow` — новая state-driven система управления серверным флоу (Lobby/PreMatch/Match/PostMatch/Transition), которая заменяет legacy `mode_vote`.

### 1.1 Каноническая точка входа
- Исходник: `csgo/addons/sourcemod/scripting/serverflow.sp`
- Корневой orchestrator: `csgo/addons/sourcemod/scripting/serverflow/serverflow.sp`
- Рабочий бинарник (должен загружаться SourceMod): `csgo/addons/sourcemod/plugins/serverflow.smx`

### 1.2 Legacy
- Legacy-исходник сохранён только как историческая справка (**not runtime path**):  
  `csgo/addons/sourcemod/scripting/serverflow/archive/mode_vote.legacy.sp`
- Legacy build script удалён из активного цикла. Единственный runtime/build entrypoint — **ServerFlow** через `build_serverflow.bat`.

### 1.3 Степень готовности (честно)
- **Сборка/запуск**: рабочие.
- **Базовый state flow и команды**: рабочие.
- **Production-polish по ТЗ** (UX depth, расширенная диагностика, часть сценарных правил): частично.

### 1.4 Документы-источники истины
- `docs/design_doc_csgo_server_plugin_ru.md`
- `docs/technical_spec_codex_csgo_plugin_ru.md`
- `docs/implementation_roadmap_file_structure_codex_csgo_plugin_ru.md`
- `docs/agents_md_codex_csgo_plugin_ru.md`
- `docs/human_centered_ux_guidelines_for_codex_csgo_plugin_ru.md`

## 2) Как собрать и запустить

## 2.1 Каноническая сборка
```bat
build_serverflow.bat
```

Скрипт:
- ищет компилятор (`spcomp.exe`, `spcomp64.exe`, `spcomp`, `spcomp64`);
- пишет лог в `build_serverflow.log`;
- при необходимости пытается авто-чинить 2 известных stale-паттерна исходников;
- собирает `serverflow.smx`;
- зеркалит артефакт в `scripting/compiled`.

Ожидаемые выходы после успешной сборки:
- `csgo/addons/sourcemod/plugins/serverflow.smx`
- `csgo/addons/sourcemod/scripting/compiled/serverflow.smx`

## 2.2 Если платформа не принимает бинарники (`.smx`)
В репозитории лежат текстовые Base64-артефакты:
- `csgo/addons/sourcemod/plugins/serverflow.smx.b64.txt`
- `csgo/addons/sourcemod/scripting/compiled/serverflow.smx.b64.txt`

Восстановление на Windows:
```bat
certutil -decode csgo\addons\sourcemod\plugins\serverflow.smx.b64.txt csgo\addons\sourcemod\plugins\serverflow.smx
certutil -decode csgo\addons\sourcemod\scripting\compiled\serverflow.smx.b64.txt csgo\addons\sourcemod\scripting\compiled\serverflow.smx
```

### Политика для PR
- В PR не добавляем бинарные файлы (`*.smx`, `spcomp*.exe`, `compile.exe/dat`).
- Для репликации артефактов между хостами используем только `.b64.txt`.
- Если в PR внезапно попали бинарники — удаляем их из индекса до создания PR.

## 2.3 Старт сервера
- `start.bat`
- `start_classic.bat`
- `start_dz.bat`

Все они должны вызывать `build_serverflow.bat` (единственный runtime/build entrypoint ServerFlow).

---

## 3) Конфиги и доступные значения

Конфиг-папка:
`csgo/addons/sourcemod/configs/serverflow/`

## 3.1 `plugin_core.cfg`
Текущие runtime-параметры (ConVar):
- `sm_serverflow_vote_duration` — длительность голосования (сек)
- `sm_serverflow_vote_cooldown` — кулдаун между голосованиями (сек)
- `sm_serverflow_vote_min_players` — минимальное число игроков для запуска голосования
- `sm_serverflow_pending_confirm_window` — окно подтверждения pending change (сек)
- `sm_serverflow_verbose` — подробность логирования (0/1)

## 3.2 `scenarios.cfg`
Обязательный формат секции сценария (текущий основной):
- `router_alias`
- `name_phrase`
- `game_type`
- `game_mode`
- `mapgroup`
- `start_map`
- `cfg_file`
- `maplist_file`
- `fallback_map`
- `rotation_mode` (`sequential`/`random`)

Дополнительно поддерживаются legacy-ключи (fallback):
- `router` (вместо `router_alias`)
- `map_list_file` / `maplist` (вместо `maplist_file`)
- `fallback` (вместо `fallback_map`)

Если `maplist_file` пуст для известных id (`dz`, `comp`, `casual`) — применяется встроенный fallback maplist.

## 3.3 `playlists.cfg`
Используется как декларативный слой плейлистов. В текущей реализации основной фактический источник карт для сценария — поле `maplist_file` у сценария + чтение соответствующего txt файла.

## 3.4 `lang_ru.cfg`, `lang_en.cfg`
Языковые ключи заготовлены, но текущий UX во многом использует технические/промежуточные сообщения. Это ожидаемо на текущей стадии миграции.

---

## 4) Команды: что работает сейчас


### Важно: как правильно вводить команды (частая ошибка)
- В **консоли**: `sm_mode`, `sm_status`, `sm_ready` ...
- В **чате**: `!mode`, `!status`, `!ready` ...

Не пишите в чат `!sm_mode`/`!sm_ready` — это неверный формат для SourceMod chat triggers.

Ниже — текущее фактическое состояние команд.

### 4.1 Игрок
- `sm_mode` — открыть меню выбора сценария. **Работает**.
- `sm_votemode` — старт голосования сценария (при соблюдении условий). **Работает**.
- `sm_status` — вывести текущий статус. **Работает**.
- `sm_status_verbose` — расширенный статус для отладки (state/pending/таймеры/next action). **Работает**.
- `sm_ready` — отметить ready. **Работает**.
- `sm_unready` — снять ready. **Работает**.
- `sm_team` — открыть Team/Ready меню (ready/unready/status) вне Transition. **Работает**.
- `sm_lobby` — попытка перейти в сценарий `lobby` (если настроен). **Работает при наличии lobby в scenarios.cfg**.
- `sm_help_serverflow` — краткая in-game справка по командам. **Работает**.

### 4.2 Админ/оператор
- `sm_queuescenario <id>` — поставить сценарий в pending. **Работает**.
- `sm_queuemap <map>` — поставить карту в pending. **Работает**.
- `sm_confirmpending` — подтвердить и применить pending. **Работает**.
- `sm_cancelpending` — отменить pending. **Работает**.
- `sm_forcescenario <id>` — админ override (через pending/transition). **Работает**.

### 4.3 Что частично/в разработке
- Полный UX-дизайн статусов для игроков (максимально дружелюбный вместо тех. сообщений).
- Полный production-уровень transition/policy/diagnostics по всей глубине 4 документов.

---

## 5) State machine и как проверять

Состояния:
- `Lobby`
- `PreMatch`
- `Match`
- `PostMatch`
- `Transition`

Дополнительно:
- pending lifecycle (`Proposed` -> `Selected` -> `Confirmed` -> `Applying`)

Что проверять тестеру:
1. В Lobby доступны выбор/голосование.
2. Pending создаётся, подтверждается, отменяется.
3. Прямого «хаотичного» apply без transition/pending нет.
4. В PreMatch ready/countdown/lock фазы не ломают состояние.

### 5.1 Важный runtime-момент при смене сценария
- `sm_queuescenario <id>` только создаёт pending.
- Применение происходит после `sm_confirmpending`.
- При apply ServerFlow выставляет `game_type` и `game_mode` сценария, затем применяет `cfg_file`/`mapgroup`, и только потом делает `changelevel`.
- Если раньше наблюдался кейс «карта DZ загрузилась, но режим остался casual», проверьте именно шаг `sm_confirmpending` и наличие в логах строк про `game_type/game_mode`.

Состояния:
- `Lobby`
- `PreMatch`
- `Match`
- `PostMatch`
- `Transition`

## 6) Подробный тест-план (для тестеров)

## 6.1 Подготовка
1. Проверить загрузку плагина:
   - `sm plugins list`
2. Проверить версию SourceMod:
   - `sm version`
3. Проверить наличие валидных maplist файлов из `scenarios.cfg`.

## 6.2 Smoke (обязательный минимум)
1. `sm_status`
2. `sm_status_verbose`
3. `sm_mode`
4. `sm_votemode`
5. `sm_queuescenario dz`
6. `sm_confirmpending`
7. `sm_queuescenario comp`
8. `sm_cancelpending`

Критерий прохождения:
- нет `SetFailState`/startup crash;
- нет неожиданных смен карт вне pending flow;
- команды дают ожидаемый ответ.

## 6.3 Границы и отрицательные кейсы
- Запустить `sm_votemode` при недостатке игроков -> должен быть отказ с причиной.
- Дважды проголосовать одним клиентом -> второй раз отказ.
- Подтвердить pending без прав -> отказ.

## 6.4 Отчёт тестера (шаблон)
- Build/commit hash
- Карта/онлайн
- Шаги
- Ожидание
- Факт
- Логи (`errors_*.log`, `build_serverflow.log`, `mode_actions.log`)

Что проверять тестеру:
1. В Lobby доступны выбор/голосование.
2. Pending создаётся, подтверждается, отменяется.
3. Прямого «хаотичного» apply без transition/pending нет.
4. В PreMatch ready/countdown/lock фазы не ломают состояние.

## 7) Диагностика ошибок: быстрый разбор

## 7.1 Ошибка загрузки `serverflow.smx` на старте
Смотрите:
- `csgo/addons/sourcemod/logs/errors_YYYYMMDD.log`
- `build_serverflow.log`

Частый кейс:
- `invalid scenario ... maplist cannot be empty`
  - проверить ключи в `scenarios.cfg`
  - проверить maplist файлы
  - учесть legacy-key fallback (см. раздел 3.2)

## 7.2 Ошибки `admin-sql-*` и таймауты
Примеры:
- `no such table: sm_overrides`
- `no such table: sm_groups`
- `no such table: sm_admins`

Это **не ServerFlow**, а SQL Admin плагины SourceMod.

Для чистого теста ServerFlow:
- временно отключить `admin-sql-prefetch.smx` / `admin-sql-threaded.smx`,
  или
- поднять корректную БД и таблицы `sm_*`.

Иначе логи будут зашумлены, а в тяжёлых случаях возможны script timeout в сторонних админ-плагинах.

## 7.3 Если `.bat` закрывается сразу
Запускать так:
```bat
cmd /k build_serverflow.bat
```
И смотреть `build_serverflow.log`.

---

## 8) Что соответствует 4 документам, а что ещё нет

### Уже соответствует (базово)
- модульная структура по слоям;
- state/pending/transition каркас;
- config-driven сценарии/карты;
- command routing.

### Частично соответствует
- UX presentation (пока частично технический);
- diagnostics/health depth;
- часть edge-case policy в transition lifecycle.

Это нормально для текущего этапа миграции, но важно при acceptance-тестах учитывать статус как **beta-stage migration**, а не финальный релиз.

---

## 9) Рекомендованный порядок действий для админа перед тестом

1. `git pull`
2. Восстановить `.smx` из `.b64.txt` (если бинарники не хранятся у вас напрямую)
3. Запустить `build_serverflow.bat`
4. Проверить `sm plugins list` и что `ServerFlow` loaded
5. Временно убрать шумные SQL admin плагины, если БД не настроена
6. Прогнать smoke-план из раздела 6.2
7. Сохранить логи и оформить отчёт по шаблону

---

## 10) Краткая памятка “что для чего”

- `serverflow.sp` — точка входа компиляции.
- `serverflow/serverflow.sp` — bootstrap/orchestration.
- `configs/serverflow/scenarios.cfg` — какие сценарии и какие карты/параметры.
- `build_serverflow.bat` — каноническая сборка + диагностика.
- `plugins/serverflow.smx` — файл, который реально должен загрузить SourceMod.
- `scripting/compiled/serverflow.smx` — зеркало артефакта для удобства.

---

## 11) UX-правило для всех тестов

Если механика работает технически, но игрок/админ не понимает:
- что происходит сейчас,
- почему это происходит,
- что произойдёт дальше,
- что можно сделать прямо сейчас,

то кейс считается **частично проваленным по UX** и должен фиксироваться в баг-репорте отдельно от runtime-ошибок.
