# Техническое ТЗ для Codex: плагин управления режимами, картами и потоком матчей для CS:GO Legacy

## 1. Назначение документа

Этот документ предназначен **именно для реализации в Codex**. Он не описывает идею “в общем”, а задаёт:

- что именно нужно реализовать;
- из каких модулей должна состоять система;
- какие данные и конфиги она должна читать;
- какие состояния и переходы обязаны существовать;
- как должны работать игроки, админ, голосования, countdown, ready-check, AFK, late join;
- какие ограничения нельзя нарушать;
- как разбить реализацию на этапы;
- какие критерии считать завершением каждого этапа.

Цель: **сделать расширяемый SourceMod-плагин для CS:GO Legacy**, который управляет сценариями сервера, картами, Prematch/Postmatch-логикой, статусами, голосованиями и безопасными переходами между состояниями.

---

# 2. Важные указания для Codex

## 2.1 Не делать

Codex **не должен**:

- реализовывать систему как набор разрозненных консольных команд без общей архитектуры;
- жёстко прошивать сценарии через множество `if/else` по всему коду;
- связывать UI напрямую с техническими cvar, именами cfg и внутренними алиасами;
- применять критические изменения режима прямо в середине активного матча без Transition Manager;
- хранить состояние сразу в нескольких несвязанных местах;
- делать основной UX как каскад сырых технических SourceMod-меню без логики состояний;
- дублировать одну и ту же бизнес-логику одновременно в menu-handlers, command-handlers и timers.

## 2.2 Нужно сделать

Codex должен реализовать систему как:

- **state machine**;
- **scenario-driven architecture**;
- **single source of truth** для текущего и следующего состояния сервера;
- **config-driven system**, где сценарии и параметры читаются из конфигов;
- **safe transition pipeline**, через который проходят все важные изменения.

---

# 3. Целевая архитектура

## 3.1 Общая модель

Плагин должен управлять сервером не на уровне “вызвали команду”, а на уровне:

- `Server State`
- `Current Scenario`
- `Queued Scenario`
- `Current Map`
- `Pending Vote`
- `Pending Change`
- `Countdown`
- `Player Lifecycle`

## 3.2 Обязательные крупные подсистемы

Реализацию нужно разделить на модули.

### Обязательные модули:
1. `StateManager`
2. `TransitionManager`
3. `ScenarioRegistry`
4. `PlaylistManager`
5. `PlayerStateManager`
6. `ReadyManager`
7. `CountdownManager`
8. `VoteManager`
9. `TeamManager`
10. `AdminOverrideManager`
11. `StatusPresenter`
12. `ConfigLoader`
13. `HealthCheck`
14. `AuditLogger`
15. `CommandRouter`

Нельзя сваливать всю систему в один большой `.sp` файл.

---

# 4. Базовые сущности

## 4.1 ServerState

Нужен enum состояний сервера:

- `STATE_LOBBY`
- `STATE_PREMATCH`
- `STATE_MATCH`
- `STATE_POSTMATCH`
- `STATE_TRANSITION`

Дополнительно нужна концепция подфаз.

## 4.2 PreMatchPhase

- `PREMATCH_SETUP`
- `PREMATCH_TEAMS`
- `PREMATCH_READY`
- `PREMATCH_LOCK`

## 4.3 PostMatchPhase

- `POSTMATCH_RESULTS`
- `POSTMATCH_DECISION`
- `POSTMATCH_CONFIRM`

## 4.4 PendingChangeState

- `PENDING_NONE`
- `PENDING_PROPOSED`
- `PENDING_SELECTED`
- `PENDING_CONFIRMED`
- `PENDING_APPLYING`

## 4.5 PlayerLifecycleState

- `PLAYER_CONNECTED`
- `PLAYER_SPECTATOR`
- `PLAYER_ELIGIBLE`
- `PLAYER_UNREADY`
- `PLAYER_READY`
- `PLAYER_ASSIGNED`
- `PLAYER_AFK`
- `PLAYER_LATEJOIN`
- `PLAYER_LOCKEDIN`

---

# 5. Scenario model

## 5.1 Что такое scenario

Scenario — это единица игровой логики.

Scenario должен описывать:

- базовый режим;
- правила команд;
- правила Prematch;
- правила Postmatch;
- map pool;
- правила ready-check;
- late join policy;
- afk policy;
- countdown behavior;
- ограничения по игрокам;
- доступность для игроков;
- доступность для админов.

## 5.2 Обязательные поля scenario

Каждый сценарий должен поддерживать поля:

- `id`
- `title`
- `description`
- `enabled`
- `base_mode`
- `base_cfg`
- `playlist_id`
- `default_map`
- `team_policy`
- `team_validation`
- `ready_policy`
- `ready_threshold_percent`
- `ready_threshold_min`
- `countdown_time`
- `lock_time`
- `late_join_policy`
- `afk_policy`
- `unassigned_policy`
- `min_players`
- `max_players`
- `allow_mode_vote`
- `allow_map_vote`
- `allow_team_select`
- `allow_ready`
- `postmatch_default_action`
- `postmatch_decision_time`
- `confirm_time`
- `visibility`
- `admin_only`

## 5.3 Примеры scenario id

- `lobby`
- `dz_solo`
- `dz_duo_auto`
- `dz_duo_open`
- `dz_trio_auto`
- `dz_trio_open`
- `comp_5v5`
- `casual_fun`

---

# 6. Playlist model

## 6.1 Playlist обязан быть отдельной сущностью

Нельзя жёстко держать список карт внутри сценария как массив в коде.

## 6.2 Поля playlist

- `id`
- `title`
- `rotation_mode`
- `repeat_protection_count`
- `maps[]`

## 6.3 Поля карты

- `map_name`
- `enabled`
- `weight`
- `min_players`
- `max_players`
- `tags`
- `cooldown_repeat`

## 6.4 Поддерживаемые режимы rotation

- `sequential`
- `random`
- `weighted_random`
- `vote_pool`

---

# 7. Single source of truth

Нужен один центральный объект runtime-session, который содержит:

- текущий `ServerState`
- текущую подфазу
- `currentScenarioId`
- `queuedScenarioId`
- `previousScenarioId`
- `currentMap`
- `queuedMap`
- `pendingChangeState`
- `pendingChangeSource`
- `pendingChangeCreatedAt`
- `activeVoteId`
- `countdownActive`
- `countdownEndsAt`
- `lockPhaseActive`
- `overrideLevel`
- `fallbackReason`

Никакой другой модуль не должен хранить альтернативную версию этих данных.

---

# 8. StateManager

## 8.1 Обязанности

StateManager отвечает за:

- текущее состояние сервера;
- смену состояний;
- проверку допустимости переходов;
- уведомление других модулей о смене состояния.

## 8.2 Разрешённые переходы

Минимально должны поддерживаться:

- `Lobby -> PreMatch`
- `PreMatch -> Match`
- `Match -> PostMatch`
- `PostMatch -> Transition`
- `Transition -> Lobby`
- `Transition -> PreMatch`

Дополнительно:
- аварийный переход в `Lobby` из любого состояния;
- safe override переход в `PostMatch` / `PreMatch` логически через очередь.

## 8.3 Недопустимые переходы

Например:
- прямой переход `Match -> Lobby` без подтверждённого hard override или fallback;
- прямой переход `Match -> PreMatch` без Transition;
- прямое применение нового сценария в обход StateManager.

---

# 9. TransitionManager

## 9.1 Это главный модуль применения изменений

Все критические изменения должны идти **только через TransitionManager**.

## 9.2 Что он делает

1. Принимает подтверждённое решение.
2. Проверяет, можно ли сейчас применить.
3. Валидирует сценарий.
4. Валидирует карту.
5. Валидирует cfg и playlist.
6. Выполняет подготовку.
7. Применяет правила.
8. Инициирует `changelevel`.
9. Обновляет runtime-state.
10. Логирует результат.

## 9.3 Категории применения

- `TRANSITION_TO_SCENARIO`
- `TRANSITION_TO_MAP`
- `TRANSITION_TO_LOBBY`
- `TRANSITION_RESTART_SCENARIO`
- `TRANSITION_EMERGENCY`

## 9.4 Безопасные точки применения

TransitionManager должен знать safe-points:

- в Lobby — можно применять сразу;
- в PreMatch — можно применять после confirm;
- в Match — нельзя применять обычную смену сценария немедленно;
- в PostMatch — можно применять после confirm;
- аварийный переход — только через override уровня emergency.

---

# 10. PlayerStateManager

## 10.1 Обязанности

Управляет логикой игрока:

- spectator / eligible / ready / AFK / late join;
- входит ли игрок в ready-pool;
- участвует ли игрок в текущем сценарии;
- зафиксирован ли игрок для текущего старта.

## 10.2 События, которые он должен обрабатывать

- player connect
- player disconnect
- player spawn
- player team change
- player activity / inactivity
- player ready
- player unready
- player assigned to squad
- player moved to spectator

## 10.3 AFK detection

AFK detection должна быть отдельной логикой, а не случайной проверкой в разных местах.

AFK-таймаут, способ реакции и исключение из ready-pool должны настраиваться.

---

# 11. ReadyManager

## 11.1 Обязанности

- считать ready-pool;
- считать ready count;
- знать, когда условия ready выполнены;
- запускать/сбрасывать countdown;
- давать читаемую причину, почему матч не может стартовать.

## 11.2 Поддерживаемые ready-политики

- `READY_ALL`
- `READY_PERCENT`
- `READY_MINIMUM`
- `READY_ADMIN_CONFIRM`
- `READY_CAPTAIN_CONFIRM`
- `READY_TIMEOUT_AUTO`

## 11.3 Ready-pool rules

ReadyManager должен использовать только тех игроков, кто:

- eligible для сценария;
- не spectator, если spectator excluded;
- не AFK, если AFK excluded;
- не late join после lock phase;
- не исключён вручную.

## 11.4 Обязательная функция объяснения

ReadyManager должен уметь вернуть строку причины, почему старт невозможен.

Примеры:
- `Недостаточно игроков: 5/8`
- `Готовность не достигнута: 6/8`
- `2 игрока не выбрали команду`
- `Идёт окно подтверждения смены сценария`

---

# 12. CountdownManager

## 12.1 Обязанности

- запуск countdown;
- отмена countdown;
- lock phase;
- уведомления игрокам;
- причины сброса.

## 12.2 Countdown запускается только если

- сценарий валиден;
- команды валидны;
- ready-condition выполнен;
- нет конфликтующего pending-change;
- нет блокировки старта.

## 12.3 Countdown должен сбрасываться, если

- новый eligible-игрок подключился в soft late join window;
- кто-то снял ready;
- команды стали невалидны;
- сценарий переопределён;
- админ поставил новый queued override.

## 12.4 Lock phase

После countdown должна быть финальная `lock phase`.

В lock phase:
- ready замораживается;
- squads замораживаются;
- late join -> spectator;
- обычные игроки больше не сбивают старт;
- только admin emergency override или fallback может прервать запуск.

---

# 13. TeamManager

## 13.1 Обязанности

- управлять формированием команд / сквадов;
- проверять валидность состава;
- поддерживать разные team policy.

## 13.2 Поддерживаемые team policy

- `TEAM_AUTO`
- `TEAM_OPEN`
- `TEAM_CAPTAIN_DRAFT`
- `TEAM_KEEP_PREVIOUS`
- `TEAM_AUTO_FILL_LEFTOVERS`

## 13.3 Team validation

Нужны два режима:

- `TEAM_VALIDATION_STRICT`
- `TEAM_VALIDATION_SOFT`

## 13.4 Что должно проверяться

- не переполнены ли команды;
- все ли обязательные игроки распределены;
- нет ли “лишних одиночек”;
- хватает ли команд для сценария;
- соответствуют ли размеры squad правилам сценария.

---

# 14. VoteManager

## 14.1 Поддерживаемые типы голосований

- vote за следующий сценарий;
- vote за следующую карту;
- vote за возврат в Lobby;
- vote за продолжение текущего сценария;
- при необходимости — vote за team mode, если сценарий разрешает.

## 14.2 Vote lifecycle

Нужен явный lifecycle:

- `VOTE_CREATED`
- `VOTE_ACTIVE`
- `VOTE_SELECTED`
- `VOTE_CONFIRMED`
- `VOTE_APPLIED`
- `VOTE_CANCELLED`
- `VOTE_EXPIRED`

## 14.3 Ограничения голосований

Обязательные параметры:

- cooldown;
- minimum players;
- quorum;
- repeat protection;
- allow during state;
- allow override by admin.

## 14.4 Tie-break behavior

Нужно поддержать настраиваемую стратегию:

- `prefer_current`
- `random_from_leaders`
- `revote`

## 14.5 Игроки должны видеть

- что vote начался;
- сколько времени осталось;
- лидирующий вариант;
- свой голос;
- итог;
- queued result;
- apply timing.

---

# 15. Pending Change model

## 15.1 Pending change — обязательная сущность

Нельзя сразу применять выбранный результат.

Нужен объект pending change с полями:

- `type`
- `source`
- `scenarioId`
- `mapName`
- `createdBy`
- `createdAt`
- `state`
- `confirmDeadline`
- `applyAtSafePoint`
- `cancelAllowed`

## 15.2 Источники pending change

- `SOURCE_PLAYER_VOTE`
- `SOURCE_ADMIN_QUEUE`
- `SOURCE_DEFAULT_POSTMATCH`
- `SOURCE_FALLBACK`

## 15.3 Стадии

- proposed
- selected
- confirmed
- applying
- applied
- cancelled

---

# 16. AdminOverrideManager

## 16.1 Уровни override

- `OVERRIDE_QUEUE`
- `OVERRIDE_SAFE`
- `OVERRIDE_EMERGENCY`

## 16.2 Что должен уметь админ

- queue next scenario;
- queue next map;
- cancel pending change;
- lock votes;
- unlock votes;
- force lobby;
- restart current scenario;
- emergency switch;
- reload configs;
- run health check.

## 16.3 Ограничения

Даже админ не должен:
- запускать несуществующий сценарий;
- выбирать несуществующую карту;
- ломать state machine;
- обходить логирование;
- создавать конфликтующие pending change без явной замены предыдущего.

---

# 17. StatusPresenter

## 17.1 Обязательные каналы видимости

Нужно реализовать 3 уровня показа состояния.

### 1. Event notifications
Короткие события в чат / уведомления.

### 2. Persistent compact status
Команда `!status` и компактный статусный вывод.

### 3. Detailed action view
Подробный экран при взаимодействии с системой.

## 17.2 Что должен уметь показать StatusPresenter

- текущее состояние;
- текущий сценарий;
- следующий сценарий;
- текущую карту;
- следующую карту;
- готовность;
- countdown;
- lock phase;
- active vote;
- pending change;
- источник решения;
- причину, почему матч не стартует.

---

# 18. CommandRouter

## 18.1 Игроковые команды

Реализовать минимум:

- `!play`
- `!team`
- `!ready`
- `!unready`
- `!status`
- `!vote`

## 18.2 Админские команды

Реализовать минимум:

- `!adminmode`
- `!forcelobby`
- `!queuescenario <id>`
- `!queuemap <map>`
- `!cancelpending`
- `!lockvotes`
- `!unlockvotes`
- `!restartscenario`
- `!healthcheck`
- `!reloadpluginconfig`

## 18.3 Важно

Команды должны вызывать модули, а не содержать в себе бизнес-логику.

---

# 19. ConfigLoader

## 19.1 Нужно реализовать трёхслойную схему конфигов

### A. `plugin_core.cfg`
Глобальные настройки.

### B. `scenarios.cfg`
Описание сценариев.

### C. `playlists.cfg` или папка `playlists/*.cfg`
Описание пулов карт.

## 19.2 Требования к конфигам

- поддерживать reload без полной перезагрузки сервера, если возможно;
- валидироваться при загрузке;
- выдавать понятные ошибки;
- иметь комментарии;
- быть пригодными для ручного редактирования.

## 19.3 Поля `plugin_core.cfg`

Обязательно поддержать:

- интервалы status update;
- default countdown;
- default lock time;
- confirm window time;
- vote cooldown;
- minimum players for vote;
- quorum;
- AFK timeout;
- late join mode default;
- fallback mode;
- chat notification toggles;
- audit log toggles.

---

# 20. HealthCheck

## 20.1 Должен проверять

- загружен ли plugin;
- загружены ли сценарии;
- загружены ли playlists;
- валидны ли карты;
- существует ли текущий сценарий;
- существует ли queued scenario;
- есть ли конфликты состояния;
- есть ли битые конфиги;
- не завис ли countdown.

## 20.2 Формат

Нужен компактный human-readable отчёт.

---

# 21. AuditLogger

## 21.1 Обязательные события для логирования

- state transitions;
- votes created / selected / confirmed / cancelled;
- admin override queued / applied / cancelled;
- countdown start / cancel / reason;
- scenario apply success / failure;
- fallback reason;
- AFK auto actions;
- late join decisions.

---

# 22. UX contract

## 22.1 Главный UX-принцип

Игрок должен видеть не технические параметры, а смысл.

Нельзя показывать игрокам в качестве основного UI такие сущности как:
- raw cfg names;
- router aliases;
- внутренние enum names;
- технические cvar.

Игрок должен видеть:
- Solo / Duo / Trio
- Авто-команды / Ручной сбор
- Следующий матч
- Сменить карту
- В Lobby
- Старт через 10 секунд
- Старт отменён: новый игрок подключился

## 22.2 Информационная прозрачность

Система обязана всегда показывать:
- где мы сейчас;
- что идёт сейчас;
- что выбрано следующим;
- когда это применится;
- можно ли ещё это изменить.

---

# 23. Поведение по состояниям

## 23.1 Lobby

Разрешено:
- выбирать сценарий;
- голосовать;
- ставить админский queue override;
- менять планы;
- сразу запускать Transition в PreMatch.

## 23.2 PreMatch

Разрешено:
- показывать preview сценария;
- формировать команды;
- делать ready-check;
- запускать countdown;
- отменять/сбрасывать старт по правилам.

## 23.3 Match

Разрешено:
- голосовать только за следующее;
- queue admin override;
- late join -> spectator.

Не разрешено:
- обычным способом ломать текущий активный матч.

## 23.4 PostMatch

Разрешено:
- показать result summary;
- дать окно выбора;
- поставить pending decision;
- confirm и перейти в Transition.

## 23.5 Transition

Разрешено:
- только применение проверенного решения.

---

# 24. Поведение после матча

## 24.1 Дефолт

Если ничего не выбрано:
- продолжить текущий сценарий;
- выбрать следующую карту текущего playlist.

## 24.2 Альтернативы

Игроки/админ могут выбрать:
- ещё один матч;
- сменить карту;
- сменить сценарий;
- Lobby.

## 24.3 Lobby не должен быть обязательным после каждой игры

Lobby — safe-state, а не обязательный экран после каждого матча.

---

# 25. Late join и countdown contract

## 25.1 Обязательное поведение

Если countdown уже идёт, и приходит новый eligible-игрок:

### Если режим soft late join:
- countdown сбрасывается;
- новый игрок учитывается;
- ready-condition пересчитывается.

### Если режим hard late join:
- игрок уходит в spectator до следующей safe-point.

## 25.2 Игрокам нужно показать причину

Пример:
- `Старт отменён: подключился новый игрок`

---

# 26. Критические ограничения реализации

## 26.1 Нельзя делать полный функционал сразу в одном коммите

Codex должен реализовывать систему по этапам.

## 26.2 Нельзя смешивать UI и бизнес-логику

UI только отображает и отправляет действия.
Все решения принимает state/transition/business layer.

## 26.3 Нельзя дублировать правила сценариев в коде

Сценарии должны быть читаемы как данные.

## 26.4 Нельзя применять режимы в обход TransitionManager

Это один из главных архитектурных запретов.

---

# 27. Этапы реализации

## Этап 1. Каркас архитектуры

Реализовать:
- runtime session store;
- enums состояний;
- StateManager;
- ScenarioRegistry;
- PlaylistManager;
- ConfigLoader;
- базовый AuditLogger.

### Критерий готовности этапа 1
- плагин загружается;
- читает конфиги;
- валидирует сценарии;
- умеет вывести базовый статус.

---

## Этап 2. State machine и TransitionManager

Реализовать:
- допустимые переходы;
- pending change model;
- transition pipeline;
- fallback to Lobby;
- базовую смену сценария и карты.

### Критерий готовности этапа 2
- можно безопасно перевести сервер из Lobby в PreMatch выбранного сценария;
- можно вернуть сервер в Lobby;
- invalid scenario/map не ломают сервер.

---

## Этап 3. Player lifecycle + ready + countdown

Реализовать:
- PlayerStateManager;
- ReadyManager;
- CountdownManager;
- AFK detection;
- soft/hard late join logic;
- lock phase.

### Критерий готовности этапа 3
- Prematch умеет ждать ready;
- countdown стартует и сбрасывается по правилам;
- late join обрабатывается корректно.

---

## Этап 4. TeamManager

Реализовать:
- TEAM_AUTO;
- TEAM_OPEN;
- базовую валидацию команд;
- unassigned-player policy.

### Критерий готовности этапа 4
- командные сценарии не стартуют при невалидных составах;
- open teams работают в Prematch.

---

## Этап 5. VoteManager + Pending/Confirm UX

Реализовать:
- vote за сценарий;
- vote за карту;
- кворум, cooldown, tie-break;
- pending selection;
- confirm window;
- apply timing.

### Критерий готовности этапа 5
- игроки могут выбирать следующее действие;
- результат голосования не применяется мгновенно;
- решение видно всем.

---

## Этап 6. Admin override

Реализовать:
- queue override;
- safe override;
- emergency override;
- cancel pending;
- vote lock.

### Критерий готовности этапа 6
- админ может безопасно переопределять ход сервера;
- игроки видят, что сделал админ;
- override не ломает state machine.

---

## Этап 7. Status/UI layer

Реализовать:
- `!status`;
- event notifications;
- compact status output;
- detailed action views для `!play` и `!vote`;
- понятные тексты.

### Критерий готовности этапа 7
- игрок всегда может понять текущий и следующий статус сервера.

---

## Этап 8. Health-check + polish

Реализовать:
- `!healthcheck`;
- расширенное логирование;
- защита от конфликтующих состояний;
- проверка зависшего countdown;
- final cleanup.

### Критерий готовности этапа 8
- админ может диагностировать состояние системы;
- основные ошибки отрабатываются безопасно.

---

# 28. Минимальный MVP

Если Codex не может сделать всё сразу, MVP должен включать:

- StateManager
- TransitionManager
- Config-driven scenarios
- Lobby -> PreMatch -> Match -> PostMatch -> Transition
- Ready-check
- Countdown + reset
- Late join handling
- Pending change
- Vote for next scenario/map
- Admin queue override
- `!status`
- fallback to Lobby

Без этих частей систему нельзя считать соответствующей концепции.

---

# 29. Что считать ошибкой реализации

Реализация считается неправильной, если:

- сценарии захардкожены в коде без нормального registry;
- смена режима идёт напрямую без TransitionManager;
- нет явного Pending/Confirm слоя;
- countdown не сбрасывается при изменении условий;
- late join обрабатывается непредсказуемо;
- игроки не видят, что сейчас происходит;
- админ ломает матч мгновенными действиями без правил;
- конфиги непонятные и без комментариев;
- система не умеет падать в Lobby при ошибке.

---

# 30. Критерий финальной готовности всей системы

Система готова, когда:

- реализована state machine;
- сценарии читаются из конфигов;
- playlists читаются из конфигов;
- работает Prematch;
- работает Postmatch;
- есть Pending / Confirm / Apply;
- late join и AFK не ломают логику;
- команды и ready-check управляются по сценарию;
- все переходы безопасны;
- админские override работают корректно;
- игроки всегда видят current + next state;
- есть fallback в Lobby;
- есть health-check;
- есть audit log;
- добавление нового сценария не требует переписывания ядра.

---

# 31. Финальная инструкция для Codex

Реализуй плагин как модульную state-driven system, а не как набор отдельных команд и меню.

Сначала построй архитектурный каркас и runtime-state, потом transition pipeline, потом player lifecycle и prematch logic, затем votes/admin override, и только после этого финальный UI-слой.

Все сценарии и правила должны быть описываемы через конфиги. Все важные изменения должны сначала становиться pending action, затем проходить confirm window, и только потом применяться через TransitionManager.

Игроки и админ всегда должны видеть:
- текущее состояние;
- следующий шаг;
- источник решения;
- время применения;
- причину, если матч пока не может стартовать.

При любой ошибке система должна предпочитать безопасный fallback в Lobby, а не зависание или хаотичное поведение.

