# PLUGIN GUIDE (RU): ServerFlow

> Этот документ — практический runbook для админов и тестеров.  
> Цель: чтобы можно было без путаницы собрать, запустить, проверить и диагностировать текущий ServerFlow.

Актуальный плагин: **ServerFlow**.

## 1) Что это за плагин и текущий статус

`ServerFlow` — новая state-driven система управления серверным флоу (Lobby/PreMatch/Match/PostMatch/Transition), которая заменяет legacy `mode_vote`.

### 1.1 Каноническая точка входа
- Исходник: `csgo/addons/sourcemod/scripting/serverflow.sp`
- Корневой orchestrator: `csgo/addons/sourcemod/scripting/serverflow/serverflow.sp`
- Рабочий бинарник (должен загружаться SourceMod): `csgo/addons/sourcemod/plugins/serverflow.smx`

### 1.2 Legacy
- Legacy-исходник сохранён как архив:  
  `csgo/addons/sourcemod/scripting/serverflow/archive/mode_vote.legacy.sp`
- `build_mode_vote.bat` оставлен только как shim, проксирует в `build_serverflow.bat`.

### 1.3 Степень готовности (честно)
- **Сборка/запуск**: рабочие.
- **Базовый state flow и команды**: рабочие.
- **Production-polish по ТЗ** (UX depth, расширенная диагностика, часть сценарных правил): частично.

---

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

## 2.3 Старт сервера
- `start.bat`
- `start_classic.bat`
- `start_dz.bat`

Все они должны вызывать `build_serverflow.bat`.

---

## 3) Конфиги и доступные значения

Конфиг-папка:
`csgo/addons/sourcemod/configs/serverflow/`

## 3.1 `plugin_core.cfg`
Текущие runtime-параметры (ConVar):
- `sm_mode_vote_duration` — длительность голосования (сек)
- `sm_mode_vote_cooldown` — кулдаун между голосованиями (сек)
- `sm_mode_vote_min_players` — минимальное число игроков для запуска голосования
- `sm_mode_pending_confirm_window` — окно подтверждения pending change (сек)
- `sm_mode_vote_verbose` — подробность логирования (0/1)

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
- В **консоли**: `sm_mode`, `sm_status`, `sm_status_verbose`, `sm_ready` ...
- В **чате**: `!mode`, `!status`, `!status_verbose`, `!ready` ...

Не пишите в чат `!sm_mode`/`!sm_ready` — это неверный формат для SourceMod chat triggers.

Ниже — текущее фактическое состояние команд.

### 4.1 Игрок
- `sm_mode` — открыть меню выбора сценария. **Работает**.
- `sm_votemode` — старт голосования сценария (при соблюдении условий). **Работает**.
- `sm_status` — компактный статус (для игрока, одна строка). **Работает**.
- `sm_status_verbose` — расширенный статус (диагностика: prematch/pending/таймеры). **Работает**.
- `sm_ready` — отметить ready. **Работает**.
- `sm_unready` — снять ready. **Работает**.
- `sm_team` — открывает Team/Ready-меню (`READY`, `UNREADY`, `Что сейчас происходит`). Доступно вне `Transition` и вне активного `Match`; при блокировке команда объясняет причину и рекомендует `sm_status`. **Работает**.
- `sm_lobby` — попытка перейти в сценарий `lobby` (если настроен). **Работает при наличии lobby в scenarios.cfg**.
- `sm_help_serverflow` — краткая in-game справка по командам. **Работает**.


### 4.1.1 Примеры вывода статуса и как их читать

**Компактный (`sm_status`)**
```
[ServerFlow] current=comp next=dz state=PreMatch next_action=wait for players ready and countdown
```
Интерпретация для тестера:
- `state=PreMatch` — матч ещё не начался, сервер в предматчевой фазе.
- `next_action=wait for players ready and countdown` — следующий ожидаемый шаг: игроки ставят ready, затем пойдёт countdown/lock.

**Расширенный (`sm_status_verbose`)**
```
[ServerFlow] current=comp next=dz state=PreMatch prematch=Ready ready=8/10 cd=0 pending=proposed apply_state=PostMatch apply_in=42 confirm=manual next_action=await admin confirm (sm_confirmpending) or cancel
```
Интерпретация для тестера:
- `ready=8/10` — ещё 2 игрока не готовы.
- `pending=proposed` + `confirm=manual` — изменение подготовлено, но не подтверждено.
- `apply_state=PostMatch` + `apply_in=42` — применить можно в PostMatch, окно подтверждения осталось ~42 сек.
- `next_action=...` — что конкретно оператору делать дальше (подтвердить или отменить pending).

### 4.2 Админ/оператор
- `sm_queuescenario <id>` — поставить сценарий в pending. **Работает**.
- `sm_queuemap <map>` — поставить карту в pending. **Работает**.
- `sm_confirmpending` — подтвердить и применить pending. **Работает**.
- `sm_cancelpending` — отменить pending. **Работает**.
- `sm_forcescenario <id>` — админ override (через pending/transition). **Работает**.

### 4.3 Частые ошибки ввода команд

| Где вводите | Неправильно | Правильно | Что произойдёт |
|---|---|---|---|
| Чат | `!sm_mode` | `!mode` | `!sm_mode` не сработает, потому что chat trigger ожидает alias без `sm_`. |
| Чат | `!sm_ready` | `!ready` | Игрок думает, что нажал ready, но сервер не меняет ready-флаг. |
| Консоль | `!mode` | `sm_mode` | `!mode` в консоли невалиден, т.к. `!`-префикс только для chat trigger. |
| Консоль | `!votemode` | `sm_votemode` | Команда не распознаётся движком как консольная команда. |

Быстрый чек для тестера:
1. Если команда вводилась в чат — используйте `!alias`.
2. Если команда вводилась в консоль клиента/сервера — используйте `sm_*`.
3. Если «ничего не произошло», первым делом проверьте, не перепутан ли формат ввода.

### 4.4 Матрица: команда -> доступные состояния -> права -> возможные причины отказа

| Команда | Доступные состояния | Права | Возможные причины отказа |
|---|---|---|---|
| `sm_mode` / `!mode` | Обычно `Lobby`, частично `PreMatch` (зависит от lock phase) | Игрок | Нет доступных сценариев; меню временно заблокировано состоянием/фазой. |
| `sm_votemode` / `!votemode` | В первую очередь `Lobby` | Игрок | Недостаточно игроков (`sm_mode_vote_min_players`); кулдаун; голосование уже активно. |
| `sm_ready` / `!ready` | `PreMatch` | Игрок | Команда вне `PreMatch`; игрок не входит в пул ready-check; состояние уже `Match`. |
| `sm_unready` / `!unready` | `PreMatch` | Игрок | Нет активного ready-окна/lock-фазы; игрок уже unready; фаза уже зафиксирована. |
| `sm_status`, `sm_status_verbose` | Любое | Игрок/админ | Обычно не отказывает; при проблемах только технические ограничения логирования. |
| `sm_queuescenario <id>` | `Lobby`, `PostMatch`, иногда `Transition` (по guard-правилам) | Админ (`Generic`/`Root`) | Неизвестный `id`; уже есть pending; запрещено текущим state guard. |
| `sm_confirmpending` | Когда есть `pending` | Админ (`Generic`/`Root`) | Нет активного pending; confirm window истёк; недостаточно прав. |
| `sm_cancelpending` | Когда есть `pending` | Админ (`Generic`/`Root`) | Нет pending для отмены; недостаточно прав. |
| `sm_forcescenario <id>` | Любое (но нежелательно в середине критичной match-фазы) | Админ (`Root`) | Неизвестный сценарий; guard блокирует небезопасный переход; нет прав. |

### 4.5 Что частично/в разработке
- Полный UX-дизайн статусов для игроков (максимально дружелюбный вместо тех. сообщений).
- Полный production-уровень transition/policy/diagnostics по всей глубине 4 документов.

---

## 5) Сценарии эксплуатации (playbook)

> Для каждого шага фиксируются: **команда**, **предусловия**, **ожидаемый ответ**, **изменение state**, **что видит игрок**, **что видит админ**.

### 5.1 Lobby -> PreMatch -> Match -> PostMatch -> Transition

| Шаг | Команда | Предусловия | Ожидаемый ответ | Изменение state | Что видит игрок | Что видит админ |
|---|---|---|---|---|---|---|
| 1 | `sm_status` | Плагин загружен, сервер в `Lobby` | Статус с `Lobby` | Нет | Текущее состояние сервера | То же + baseline в логах |
| 2 | `sm_mode` / `!mode` | В `Lobby`, меню разрешено | Открывается меню сценариев | Нет | Список доступных сценариев | Факт открытия меню и выбор игрока |
| 3 | `sm_votemode` / голосование в меню | Достаточно игроков, нет кулдауна | Голосование запущено/зафиксирован выбор | Обычно нет (до apply) | Таймер и результат голосования | Аудит старта/результата голосования |
| 4 | `sm_queuescenario <id>` | У админа есть право, выбран валидный `id` | Создан pending change | Обычно без смены до confirm | Уведомление о запланированной смене | Pending запись + guard-проверки |
| 5 | `sm_confirmpending` | Pending активен, окно подтверждения не истекло | Pending подтверждён и применён | `Transition` -> `PreMatch` | Сообщение о применении сценария, подготовка матча | Подробный маршрут apply в audit/log |
| 6 | `sm_ready` (все игроки) | Сервер в `PreMatch`, активен ready-check | Готовность принята, стартует countdown/lock | `PreMatch` -> `Match` после lock/countdown | Countdown и затем старт матча | Маркеры `lock->match applied` |
| 7 | Игровые события `round_end`/`game_end` | Сервер в `Match` | Матч завершён | `Match` -> `PostMatch` | Сообщение об окончании матча | Маркеры `match->postmatch` |
| 8 | `sm_queuescenario <id2>` + `sm_confirmpending` | В `PostMatch`, нужен следующий сценарий | Создан и применён новый pending | `PostMatch` -> `Transition` -> `PreMatch/Lobby` по guard | Подготовка к следующей сессии | Лог перехода и новая активная конфигурация |

### 5.2 Pending: create/select/confirm/cancel/timeout

| Шаг | Команда | Предусловия | Ожидаемый ответ | Изменение state | Что видит игрок | Что видит админ |
|---|---|---|---|---|---|---|
| 1 | `sm_queuescenario dz` | Админ, валидный сценарий, pending отсутствует | `pending` создан | Обычно без смены state до confirm | Уведомление о запланированной смене | Кто поставил pending, какой `id` |
| 2 | `sm_status_verbose` | Pending существует | Статус показывает pending и окно подтверждения | Нет | Косвенно: сообщение о pending | Детали confirm window/time left |
| 3A | `sm_confirmpending` | Окно подтверждения активно | Pending принят и применён | `... -> Transition -> целевой state` | Сообщение о начале/успехе применения | Полный apply pipeline в логах |
| 3B | `sm_cancelpending` | Pending активен | Pending отменён | Нет/возврат в исходный state | Сообщение об отмене | Причина/инициатор отмены |
| 3C | Ожидание timeout | Никто не подтвердил pending до дедлайна | Pending истекает автоматически | Нет (кроме очистки pending) | Уведомление о таймауте (если verbose) | Запись timeout в audit |

### 5.3 Force scenario

| Шаг | Команда | Предусловия | Ожидаемый ответ | Изменение state | Что видит игрок | Что видит админ |
|---|---|---|---|---|---|---|
| 1 | `sm_forcescenario comp` | Админ с `Root`, сценарий существует | Принят force-запрос | Может сразу инициировать `Transition` | Резкое уведомление о принудительной смене | Запись о forced path |
| 2 | `sm_status_verbose` | Force выполнен/в процессе | Статус отражает актуальный state и сценарий | Фиксация нового рабочего state | Игрок видит новое состояние сервера | Админ сверяет отсутствие guard-denied/ошибок |

### 5.4 Late join / ready reset / countdown cancel

| Шаг | Команда/действие | Предусловия | Ожидаемый ответ | Изменение state | Что видит игрок | Что видит админ |
|---|---|---|---|---|---|---|
| 1 | Late join (игрок подключается в `PreMatch`) | Уже идёт ready-check | Новый игрок получает текущий контекст ready-фазы | Обычно остаётся `PreMatch` | Видит текущий статус ready/countdown | Видит изменение состава ready-пула |
| 2 | `sm_unready` новым/любым игроком | Countdown уже начался, но lock не финализирован | Countdown отменяется/откатывается по правилам ready-менеджера | `PreMatch` сохраняется | Сообщение об отмене countdown | Лог причины reset/cancel |
| 3 | `sm_ready` повторно всеми | После reset все снова ready | Countdown запускается заново | `PreMatch` -> `Match` после lock | Новый countdown и старт матча | Подтверждение корректного повторного цикла |

---

## 6) State machine и как проверять

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

### 6.1 Как из pre-match запускается match (пошагово)
1. **Условие входа:** сервер находится в `PreMatch`, активен `lock phase`, и `ReadyManager` подтверждает, что все игроки из пула готовы.
2. **Кто инициирует:** `CountdownManager_ForceLock` завершает countdown и вызывает отдельный orchestrator-хук `Countdown_OrchestrateAfterLock`, который делегирует переход в `Transition_TryEnterMatchFromLock`.
3. **Как подтверждается переход:** runtime-пайплайн проверяет guard-условия и вызывает `State_EnterMatch` только через `runtime/transition_manager.inc` + `runtime/state_manager.inc`; если guard не проходит — в аудит пишется `flow lock->match guard_denied`.
4. **Что увидит игрок:** чат-нотификации lock/match и детерминированный audit-маршрут (`lock->match applied` или `guard_denied`) для тестера в `mode_actions.log`.

### 6.2 Как match завершается в postmatch
1. **Условие входа:** сервер в `Match`.
2. **Кто инициирует:** `integrations/game_events.inc` по игровым событиям `round_end`, `cs_win_panel_match`, `game_end` вызывает orchestrator `Transition_TryEnterPostMatchFromMatch`.
3. **Как подтверждается переход:** guard проверяет текущее состояние и только затем вызывает `State_EnterPostMatch` через runtime-менеджеры; при отказе фиксируется `flow match->postmatch guard_denied`.
4. **Что увидит игрок:** смену state-уведомления и аудируемую причину маршрута в логах.

Состояния:
- `Lobby`
- `PreMatch`
- `Match`
- `PostMatch`
- `Transition`

## 7) Подробный тест-план (для тестеров)

## 7.1 Подготовка
1. Проверить загрузку плагина:
   - `sm plugins list`
2. Проверить версию SourceMod:
   - `sm version`
3. Проверить наличие валидных maplist файлов из `scenarios.cfg`.

## 7.2 Smoke (обязательный минимум)
1. `sm_status`
2. `sm_mode`
3. `sm_votemode`
4. `sm_queuescenario dz`
5. `sm_confirmpending`
6. `sm_queuescenario comp`
7. `sm_cancelpending`

Критерий прохождения:
- нет `SetFailState`/startup crash;
- нет неожиданных смен карт вне pending flow;
- команды дают ожидаемый ответ.

## 7.3 Границы и отрицательные кейсы
- Запустить `sm_votemode` при недостатке игроков -> должен быть отказ с причиной.
- Дважды проголосовать одним клиентом -> второй раз отказ.
- Подтвердить pending без прав -> отказ.

## 7.4 Отчёт тестера (шаблон)
- Build/commit hash
- Карта/онлайн
- Шаги
- Ожидание
- Факт
- Логи (`errors_*.log`, `build_serverflow.log`, `mode_actions.log`)

---

## 8) Диагностика ошибок: быстрый разбор

### 8.0 Быстрая матрица: симптом -> причина -> fallback поведение -> действие админа

| Симптом | Причина | Fallback поведение плагина | Действие админа |
|---|---|---|---|
| В логах `validation failed: no scenarios loaded` | `scenarios.cfg` не читается/пустой | Критичная ошибка, плагин не может продолжать работу | Проверить путь `addons/sourcemod/configs/serverflow/scenarios.cfg`, синтаксис KV, права доступа |
| В логах `validation degraded: scenario ... has empty playlist` | Для части сценариев недоступен `maplist_file`/в файле нет валидных карт | Плагин остаётся жив, включает **degraded mode**, ограничивая ротацию безопасным пулом | Восстановить maplist-файл, проверить имена карт и `IsMapValid` |
| Для `dz/comp/casual` карта не подхватывается из файла | maplist-файл отсутствует или не открылся | Используется предсказуемый fallback pool (`dz_blacksite` / `de_dust2` / `de_mirage`) | Починить файл maplist и сверить с `scenarios.cfg`/`playlists.cfg` |
| В health-логе статус `status=2` (degraded) | Конфиг валиден частично, активирован безопасный режим | Плагин работает в ограниченном режиме без падения | Проверить `g_ModeMapCacheError` по сценариям и устранить деградационные причины |

## 8.1 Ошибка загрузки `serverflow.smx` на старте
Смотрите:
- `csgo/addons/sourcemod/logs/errors_YYYYMMDD.log`
- `build_serverflow.log`

Частый кейс:
- `invalid scenario ... maplist cannot be empty`
  - проверить ключи в `scenarios.cfg`
  - проверить maplist файлы
  - учесть legacy-key fallback (см. раздел 3.2)

## 8.2 Ошибки `admin-sql-*` и таймауты
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

## 8.3 Если `.bat` закрывается сразу
Запускать так:
```bat
cmd /k build_serverflow.bat
```
И смотреть `build_serverflow.log`.

## 8.4 Troubleshooting: шумные не-ServerFlow логи

Часто при тестах рядом с полезными логами ServerFlow идут инфраструктурные сообщения, которые не относятся к логике state machine.

| Шумный лог | Это про что | Влияет на ServerFlow? | Что делать тестеру |
|---|---|---|---|
| `admin-sql-*`, `no such table: sm_*`, SQL timeout | Инфраструктура админки SourceMod/БД | Обычно **нет**, если ServerFlow загружен и его команды проходят | Помечать как infra issue и не смешивать с дефектом плагина |
| `Unknown client cvar`, `FCVAR_*`, client cvar warnings | Клиентские ограничения/античит/проверка cvar | Обычно **нет** | Не блокировать тест-кейс по ServerFlow, если state-переходы корректны |
| `material ... not found`, `model ... missing`, precache warnings | Ресурсы карты/контент сервера | Косвенно, только если ломается загрузка карты | Проверить контент-пак/fastdl и вынести в отдельный инфраструктурный баг |
| Ошибки сторонних `.smx` | Другой плагин | Нет (пока не доказано обратное) | Выделять отдельным блоком в отчёте: `third-party plugin noise` |

Минимальный критерий «похоже на баг ServerFlow»:
1. Проблема воспроизводится на командах ServerFlow (`sm_mode`, `sm_votemode`, pending-команды).
2. Есть корреляция с `mode_actions.log` и state transition-событиями.
3. Симптом относится к переходам/guard/pending, а не к БД админки или клиентским ресурсам.

---

## 9) Что соответствует 4 документам, а что ещё нет

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

## 10) Рекомендованный порядок действий для админа перед тестом

1. `git pull`
2. Восстановить `.smx` из `.b64.txt` (если бинарники не хранятся у вас напрямую)
3. Запустить `build_serverflow.bat`
4. Проверить `sm plugins list` и что `ServerFlow` loaded
5. Временно убрать шумные SQL admin плагины, если БД не настроена
6. Прогнать smoke-план из раздела 7.2
7. Сохранить логи и оформить отчёт по шаблону

---

## 11) Краткая памятка “что для чего”

- `serverflow.sp` — точка входа компиляции.
- `serverflow/serverflow.sp` — bootstrap/orchestration.
- `configs/serverflow/scenarios.cfg` — какие сценарии и какие карты/параметры.
- `build_serverflow.bat` — каноническая сборка + диагностика.
- `plugins/serverflow.smx` — файл, который реально должен загрузить SourceMod.
- `scripting/compiled/serverflow.smx` — зеркало артефакта для удобства.
