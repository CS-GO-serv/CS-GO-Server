# PLUGIN GUIDE (RU): ServerFlow

## 1. Что сейчас актуально

Актуальный плагин: **ServerFlow**.

- Точка входа компиляции: `csgo/addons/sourcemod/scripting/serverflow.sp`
- Корневой orchestration-файл: `csgo/addons/sourcemod/scripting/serverflow/serverflow.sp`
- Целевой бинарник: `csgo/addons/sourcemod/plugins/serverflow.smx`

Legacy ModeVote сохранён только как архив исходников:
`csgo/addons/sourcemod/scripting/serverflow/archive/mode_vote.legacy.sp`

## 2. Сборка и запуск

### 2.1 Каноническая сборка
```bat
build_serverflow.bat
```

После успешной сборки скрипт кладёт бинарник в **два места**:
- `csgo/addons/sourcemod/plugins/serverflow.smx` (рабочий путь загрузки плагина)
- `csgo/addons/sourcemod/scripting/compiled/serverflow.smx` (зеркальный артефакт для удобства)

Если запускаете `.bat` двойным кликом и окно быстро закрывается, откройте лог:
- `build_serverflow.log` в корне репозитория.

Рекомендуемый запуск для диагностики:
```bat
cmd /k build_serverflow.bat
```

Если в `build_serverflow.log` видите ошибки вида:
- `undefined symbol "Path_Game"` в `config_playlists.inc`
- `reference is redundant` для `PendingChange &...` в `transition_manager.inc`

значит на сервере лежит **устаревшая копия исходников**.

`build_serverflow.bat` теперь пытается автоматически исправить эти 2 паттерна перед компиляцией (создавая `.bak` рядом с файлами):
- `config_playlists.inc.bak`
- `transition_manager.inc.bak`

Если автоматический фикс не помог, обновите исходники из репозитория (`git pull`) и повторите сборку.

### 2.2 Legacy shim (только совместимость)
```bat
build_mode_vote.bat
```
(проксирует вызов в `build_serverflow.bat`)

### 2.3 Старт
- `start.bat`
- `start_classic.bat`
- `start_dz.bat`

Все стартовые скрипты используют `build_serverflow.bat`.

---

## 3. Конфиги

ServerFlow-конфиги находятся в `csgo/addons/sourcemod/configs/serverflow/`:
- `plugin_core.cfg`
- `scenarios.cfg`
- `playlists.cfg`
- `lang_ru.cfg`
- `lang_en.cfg`
- `examples/scenarios.example.cfg`
- `examples/playlists.example.cfg`

---

## 4. Архитектурный источник истины

Проверка требований и приоритетов:
1. `docs/design_doc_csgo_server_plugin_ru.md`
2. `docs/technical_spec_codex_csgo_plugin_ru.md`
3. `docs/implementation_roadmap_file_structure_codex_csgo_plugin_ru.md`
4. `docs/agents_md_codex_csgo_plugin_ru.md`

---

## 5. Текущий статус готовности

### 5.1 Что уже готово
- Модульная структура `serverflow/` по слоям (core/runtime/config/domain/presentation/commands/diagnostics/integrations).
- Единая точка входа для сборки/запуска переключена на ServerFlow.
- Базовые компоненты runtime (Session/State/Pending/Transition), конфиг-загрузка сценариев/плейлистов, command routing.
- `serverflow.sp` компилируется на SourceMod 1.12 (ошибок компиляции нет).

### 5.2 Что ещё в процессе (по 4 документам)
- Доведение state-machine и transition-policy до полного объёма ТЗ.
- Полный pending/confirm/apply UX-пайплайн в игровом интерфейсе.
- Расширение диагностик и health-check до полного покрытия спецификации.
- Полировка presentation слоя (чтобы игрок видел игровой flow, а не тех. детали).

---

## 6. Инструкция для тестеров (подробно)

> Цель тестирования: подтвердить, что ServerFlow работает как state-driven система и не ломает переходы.

### 6.1 Подготовка окружения
1. Запустить сервер с SourceMod 1.12+.
2. Убедиться, что `serverflow.smx` загружается:
   - в серверной консоли: `sm plugins list`
3. Проверить, что в `configs/serverflow/scenarios.cfg` есть валидные сценарии и maplist-файлы.
4. Проверить, что карты из maplist реально существуют на сервере.

### 6.2 Smoke-test (обязательный минимум)
1. Открыть статус:
   - `sm_status`
2. Открыть выбор сценария:
   - `sm_mode`
3. Запустить голосование:
   - `sm_votemode`
4. Выдать pending и подтвердить:
   - `sm_queuescenario <id>`
   - `sm_confirmpending`
5. Проверить отмену pending:
   - `sm_queuescenario <id>`
   - `sm_cancelpending`

Ожидаемое поведение:
- нет runtime-ошибок;
- действия проходят через pending/confirm;
- в чате видны понятные уведомления;
- переходы не ломают сервер.

### 6.3 Тест state-machine
Проверить разрешённость действий в разных состояниях:
- в `Lobby`: выбор сценария/голосование доступны;
- в `PreMatch`: ready/unready и countdown работают;
- в `Transition`: запрещённые действия блокируются с понятной причиной.

Команды:
- `sm_ready`, `sm_unready`, `sm_status`

Ожидаемое:
- при нарушении правил состояние не повреждается;
- вместо silent fail есть явный deny reason.

### 6.4 Тест pending/confirm/apply
1. Создать pending через админ-команду.
2. Проверить таймаут окна подтверждения.
3. Подтвердить pending и проверить apply через TransitionManager.
4. Повторить с отменой pending.

Ожидаемое:
- apply не происходит напрямую из UI/команд без confirm;
- после timeout pending очищается;
- при ошибке есть контролируемый fallback и лог.

### 6.5 Тест голосования
1. Запустить голосование при достаточном числе игроков.
2. Проголосовать разными клиентами.
3. Проверить winner -> pending (а не мгновенный хаотичный apply).

Ожидаемое:
- cooldown/min players соблюдаются;
- повторное голосование одним клиентом блокируется;
- результат предсказуем и прозрачен.

### 6.6 Тест override
1. Выполнить `sm_forcescenario <id>` под админом.
2. Проверить, что pending корректно заменяется/отменяется и система остаётся валидной.

### 6.7 Логи, которые нужно приложить к отчёту
- `csgo/addons/sourcemod/logs/` (ошибки/предупреждения)
- `addons/sourcemod/logs/mode_actions.log` (audit trail)

Формат отчёта тестера:
- Build/commit
- Конфиг (какие scenarios/playlists)
- Шаги воспроизведения
- Факт/ожидание
- Логи/скриншоты/демо

---

## 7. Команды для теста и эксплуатации

Игрок:
- `sm_mode`
- `sm_votemode`
- `sm_status`
- `sm_ready`
- `sm_unready`

Админ:
- `sm_queuescenario <id>`
- `sm_queuemap <map>`
- `sm_confirmpending`
- `sm_cancelpending`
- `sm_forcescenario <id>`

---

## 8. Важные замечания

1. Официальная точка входа — только `serverflow.sp`.
2. Legacy `mode_vote` не должен блокировать сборку/тест нового плагина.
3. Если запускается массовая компиляция всех `.sp`, исключайте legacy-файлы из CI/локального пайплайна.
