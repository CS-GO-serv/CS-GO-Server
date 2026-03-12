# Implementation Roadmap + File Structure для Codex: CS:GO Legacy server flow plugin

## 1. Назначение документа

Этот документ нужен как **практический план реализации**.

Если предыдущий документ объяснял:
- что должна делать система;
- какие у неё есть состояния и правила;
- какие ограничения важны,

то этот документ объясняет:
- **как именно раскладывать код по файлам**;
- **в каком порядке реализовывать модули**;
- **что в каком файле должно лежать**;
- **какие данные, enum, struct и API должны существовать**;
- **как избежать архитектурной каши**.

Цель: чтобы Codex или любой разработчик мог открыть этот документ и последовательно собрать проект без хаоса.

---

# 2. Общий принцип структуры проекта

Проект нужно строить как **модульный SourceMod plugin**, а не как один огромный `.sp` файл.

## 2.1 Главные правила структуры

1. Один файл = одна понятная ответственность.
2. Бизнес-логика не должна жить в command handlers.
3. UI не должен принимать решения за систему.
4. Переходы состояний идут только через StateManager и TransitionManager.
5. Конфиги читаются через отдельный ConfigLoader.
6. Scenario, Playlist и Runtime Session должны быть отдельными сущностями.
7. Нельзя дублировать одно и то же состояние в нескольких местах.

---

# 3. Рекомендуемая структура папок

```text
addons/sourcemod/
  scripting/
    serverflow/
      serverflow.sp
      core/
        constants.inc
        enums.inc
        types.inc
        globals.inc
        forwards.inc
        helpers.inc
        debug.inc
      runtime/
        session_store.inc
        state_manager.inc
        transition_manager.inc
        pending_change.inc
      config/
        config_loader.inc
        config_core.inc
        config_scenarios.inc
        config_playlists.inc
        config_validation.inc
      domain/
        scenario_registry.inc
        playlist_manager.inc
        player_state_manager.inc
        ready_manager.inc
        countdown_manager.inc
        team_manager.inc
        vote_manager.inc
        admin_override_manager.inc
      presentation/
        status_presenter.inc
        chat_notifier.inc
        menu_play.inc
        menu_vote.inc
        menu_team.inc
        menu_admin.inc
      commands/
        command_router.inc
        cmd_player.inc
        cmd_admin.inc
      diagnostics/
        audit_logger.inc
        health_check.inc
        state_dump.inc
      integrations/
        game_events.inc
        map_control.inc
        timers.inc
        client_hooks.inc
        afk_monitor.inc
  configs/
    serverflow/
      plugin_core.cfg
      scenarios.cfg
      playlists.cfg
      lang_ru.cfg
      lang_en.cfg
      examples/
        scenarios.example.cfg
        playlists.example.cfg
```

Эта структура может быть слегка адаптирована под ваш стиль, но логика разбиения должна остаться такой же.

---

# 4. Главный входной файл

## 4.1 `serverflow.sp`

Это корневой файл плагина.

### Что он должен делать
- подключать все `.inc` модули;
- регистрировать Plugin info;
- вызывать bootstrap при старте;
- инициализировать runtime store;
- грузить конфиги;
- регистрировать команды;
- подписываться на события;
- запускать health checks на старте;
- логировать успешный boot.

### Чего он не должен делать
- содержать бизнес-логику ready-check;
- содержать бизнес-логику голосований;
- содержать командную логику прямо внутри себя;
- содержать десятки обработчиков меню;
- вручную решать, когда менять карту.

### Пример ответственности
`serverflow.sp` = только orchestration/bootstrap.

---

# 5. Core layer

Core layer содержит базовые типы и служебную инфраструктуру.

## 5.1 `core/constants.inc`

Здесь должны лежать:
- имена конфигов;
- лимиты строк;
- лимиты массивов;
- дефолтные значения fallback;
- внутренние ключи для session data;
- фиксированные timeout/guard-константы, если они не вынесены в конфиги.

## 5.2 `core/enums.inc`

Здесь объявляются все enum:
- ServerState
- PreMatchPhase
- PostMatchPhase
- PendingChangeState
- PlayerLifecycleState
- ReadyPolicy
- TeamPolicy
- TeamValidationMode
- VoteType
- VoteState
- OverrideLevel
- LateJoinPolicy
- AfkPolicy
- PostMatchDefaultAction
- RotationMode
- TransitionType
- HealthStatus

Важно: enum должны быть централизованы здесь, а не размазаны по разным модулям.

## 5.3 `core/types.inc`

Здесь описываются основные data structures.

Нужно определить типы/структуры для:
- RuntimeSession
- ScenarioDefinition
- PlaylistDefinition
- PlaylistMapEntry
- PendingChange
- VoteRecord
- PlayerRuntimeState
- HealthReport

Если часть структур не может быть удобно выражена как enum struct, нужно хотя бы сделать единообразные ADT-модели/обёртки.

## 5.4 `core/globals.inc`

Здесь хранятся только truly-global runtime объекты:
- current session store;
- registries;
- config caches;
- global handles timers;
- loaded menus cache, если нужно.

Важно: globals не должны становиться мусорным складом.

## 5.5 `core/forwards.inc`

Здесь объявляются межмодульные callback/forward контракты.

Например:
- OnServerStateChanged
- OnScenarioQueued
- OnPendingChangeConfirmed
- OnCountdownStarted
- OnCountdownCancelled
- OnVoteCompleted
- OnOverrideQueued
- OnHealthCheckFinished

## 5.6 `core/helpers.inc`

Утилиты общего назначения:
- форматирование строк;
- safe compare;
- clamp/normalize;
- map/scenario existence helpers;
- state-to-text conversion;
- timestamp helpers.

## 5.7 `core/debug.inc`

Отладочные функции:
- debug log wrappers;
- verbose dump of current session;
- safe asserts / guard messages.

---

# 6. Runtime layer

Runtime layer — сердце state machine.

## 6.1 `runtime/session_store.inc`

### Назначение
Это **единственный источник правды** о текущем runtime-состоянии сервера.

### Должен хранить
- current state;
- current prematch/postmatch phase;
- current scenario;
- queued scenario;
- previous scenario;
- current map;
- queued map;
- pending change;
- active vote;
- countdown state;
- lock phase state;
- override level;
- fallback reason;
- timestamps.

### Обязательные функции
- Session_Init()
- Session_ResetToLobby()
- Session_SetState()
- Session_GetState()
- Session_SetCurrentScenario()
- Session_SetQueuedScenario()
- Session_SetPendingChange()
- Session_ClearPendingChange()
- Session_SetCountdown()
- Session_ClearCountdown()
- Session_SetLockPhase()
- Session_IsLockPhase()
- Session_DebugDump()

## 6.2 `runtime/state_manager.inc`

### Назначение
Отвечает за allowed transitions между состояниями.

### Обязательные функции
- State_CanTransition(from, to)
- State_TransitionTo(newState, reason)
- State_EnterLobby()
- State_EnterPreMatch()
- State_EnterMatch()
- State_EnterPostMatch()
- State_EnterTransition()
- State_GetCurrentPhaseText()

### Важное условие
Только этот модуль имеет право официально менять server state.

## 6.3 `runtime/pending_change.inc`

### Назначение
Хранит и управляет lifecycle queued decision.

### Обязательные функции
- Pending_Create(...)
- Pending_Select(...)
- Pending_Confirm()
- Pending_Cancel(reason)
- Pending_ApplyStart()
- Pending_ApplyComplete()
- Pending_IsCancelable()
- Pending_GetSummary()

## 6.4 `runtime/transition_manager.inc`

### Назначение
Применяет решения безопасно.

### Обязательные функции
- Transition_QueueScenario()
- Transition_QueueMap()
- Transition_QueueLobby()
- Transition_ConfirmPending()
- Transition_ApplyPending()
- Transition_ValidateScenario()
- Transition_ValidateMap()
- Transition_ExecuteScenario()
- Transition_ExecuteMapChange()
- Transition_FallbackToLobby(reason)

### Важнейшее правило
Ни один другой модуль не должен менять сценарий или карту напрямую.

---

# 7. Config layer

## 7.1 `config/config_loader.inc`

### Назначение
Главная точка загрузки конфигов.

### Обязательные функции
- Config_LoadAll()
- Config_ReloadAll()
- Config_LoadCore()
- Config_LoadScenarios()
- Config_LoadPlaylists()
- Config_ValidateAll()

## 7.2 `config/config_core.inc`

### Назначение
Чтение и хранение глобальных параметров.

### Должен поддерживать поля
- default countdown;
- default lock time;
- confirm time;
- vote cooldown;
- vote quorum;
- minimum players for vote;
- AFK timeout;
- default late join behavior;
- fallback scenario;
- chat visibility toggles;
- debug verbosity.

## 7.3 `config/config_scenarios.inc`

### Назначение
Парсинг сценариев в registry.

### Обязательные функции
- Scenarios_LoadFromConfig()
- Scenarios_ValidateEntry()
- Scenarios_Register()
- Scenarios_FindById()

## 7.4 `config/config_playlists.inc`

### Назначение
Парсинг playlist и карт.

### Обязательные функции
- Playlists_LoadFromConfig()
- Playlists_ValidateEntry()
- Playlists_FindById()
- Playlists_FindMap()

## 7.5 `config/config_validation.inc`

### Назначение
Отдельный слой проверки консистентности конфигов.

### Проверять
- что scenario ссылается на существующий playlist;
- что default_map входит в playlist или валиден отдельно;
- что min/max players логичны;
- что ready policy и thresholds не конфликтуют;
- что значения enum валидны;
- что fallback scenario существует.

---

# 8. Domain layer

Здесь живёт основная бизнес-логика.

## 8.1 `domain/scenario_registry.inc`

### Назначение
Registry сценариев в памяти.

### Обязательные функции
- Scenario_Register(def)
- Scenario_Exists(id)
- Scenario_Get(id)
- Scenario_GetAllVisible()
- Scenario_IsPlayableForClients()
- Scenario_IsAdminOnly()

## 8.2 `domain/playlist_manager.inc`

### Назначение
Работа с пулами карт.

### Обязательные функции
- Playlist_GetNextMap(scenarioId)
- Playlist_GetVotePool(scenarioId)
- Playlist_IsMapAllowed(scenarioId, map)
- Playlist_RecordMapPlayed(map)
- Playlist_FilterByPlayerCount()

## 8.3 `domain/player_state_manager.inc`

### Назначение
Хранит player runtime state.

### На игрока должны храниться
- connected;
- spectator;
- eligible;
- ready;
- assignedTeamId / squadId;
- afk;
- lateJoin;
- lockedIn;
- lastActivityTime.

### Обязательные функции
- PlayerState_InitClient(client)
- PlayerState_ResetClient(client)
- PlayerState_SetReady(client, bool)
- PlayerState_SetAFK(client, bool)
- PlayerState_SetLateJoin(client, bool)
- PlayerState_SetAssigned(client, team)
- PlayerState_IsEligible(client)
- PlayerState_IsInReadyPool(client)
- PlayerState_MoveToSpectator(client, reason)

## 8.4 `domain/ready_manager.inc`

### Назначение
Логика старта по готовности.

### Обязательные функции
- Ready_Recalculate()
- Ready_GetPoolCount()
- Ready_GetReadyCount()
- Ready_IsSatisfied()
- Ready_GetFailureReason()
- Ready_SetClientReady()
- Ready_SetClientUnready()

### Важно
ReadyManager не должен запускать карту напрямую. Он только решает, выполнены ли условия.

## 8.5 `domain/countdown_manager.inc`

### Назначение
Управляет countdown и lock phase.

### Обязательные функции
- Countdown_Start(seconds, reason)
- Countdown_Cancel(reason)
- Countdown_Reset(reason)
- Countdown_IsActive()
- Countdown_EnterLockPhase()
- Countdown_IsLockPhase()
- Countdown_GetRemaining()

## 8.6 `domain/team_manager.inc`

### Назначение
Логика squads/teams.

### Обязательные функции
- Team_ResetAll()
- Team_AssignClient(client, squadId)
- Team_AutoAssignAll()
- Team_ValidateCurrent()
- Team_GetValidationReason()
- Team_AutoFillLeftovers()
- Team_LockAssignments()

## 8.7 `domain/vote_manager.inc`

### Назначение
Управление голосованиями.

### Обязательные функции
- Vote_Create(type, options)
- Vote_Start()
- Vote_Cast(client, option)
- Vote_Revoke(client)
- Vote_Close()
- Vote_GetLeader()
- Vote_GetSummary()
- Vote_ApplyResultToPending()
- Vote_CanStartNow(reason)

## 8.8 `domain/admin_override_manager.inc`

### Назначение
Безопасные действия админа.

### Обязательные функции
- Admin_QueueScenario(id)
- Admin_QueueMap(map)
- Admin_ForceLobby()
- Admin_RestartScenario()
- Admin_CancelPending()
- Admin_SetVoteLock(bool)
- Admin_EmergencyOverride(...)

---

# 9. Presentation layer

Presentation слой ничего не решает. Он только показывает и собирает ввод.

## 9.1 `presentation/status_presenter.inc`

### Назначение
Формирует человеко-понятное представление состояния.

### Обязательные функции
- Status_ShowToClient(client)
- Status_BroadcastCompact()
- Status_GetCompactText()
- Status_GetDetailedText()
- Status_GetReasonWhyBlocked()

## 9.2 `presentation/chat_notifier.inc`

### Назначение
Все короткие уведомления.

### Обязательные функции
- Notify_StateChanged()
- Notify_VoteStarted()
- Notify_VoteLeaderChanged()
- Notify_PendingSelected()
- Notify_PendingConfirmed()
- Notify_CountdownStarted()
- Notify_CountdownCancelled()
- Notify_AdminOverride()
- Notify_Fallback()

## 9.3 `presentation/menu_play.inc`

### Назначение
UI для обычных игроков: выбрать, что играть дальше.

### Должен уметь
- показать сценарии;
- показать summary сценария;
- инициировать vote/pending action;
- не содержать бизнес-логики.

## 9.4 `presentation/menu_vote.inc`

### Назначение
UI текущего голосования.

### Должен уметь
- показать варианты;
- показать текущий голос;
- позволить переголосовать;
- показать время до закрытия.

## 9.5 `presentation/menu_team.inc`

### Назначение
UI распределения по сквадам.

### Должен уметь
- показать доступные squad;
- вступить в squad;
- выйти из squad;
- показать заполненность.

## 9.6 `presentation/menu_admin.inc`

### Назначение
UI админских действий.

### Должен уметь
- queue scenario;
- queue map;
- cancel pending;
- force lobby;
- restart scenario;
- lock/unlock votes;
- health check.

---

# 10. Commands layer

## 10.1 `commands/command_router.inc`

### Назначение
Регистрирует команды и роутит их в нужные модули.

### Не должен делать
- решать голосования;
- менять состояние сервера;
- парсить сценарии вручную в куче мест.

## 10.2 `commands/cmd_player.inc`

### Команды
- `!play`
- `!team`
- `!ready`
- `!unready`
- `!status`
- `!vote`

## 10.3 `commands/cmd_admin.inc`

### Команды
- `!adminmode`
- `!forcelobby`
- `!queuescenario`
- `!queuemap`
- `!cancelpending`
- `!lockvotes`
- `!unlockvotes`
- `!restartscenario`
- `!healthcheck`
- `!reloadpluginconfig`

---

# 11. Diagnostics layer

## 11.1 `diagnostics/audit_logger.inc`

### Назначение
Логирует ключевые события.

### Логировать
- state transitions;
- pending changes;
- vote lifecycle;
- countdown lifecycle;
- admin overrides;
- transition apply success/failure;
- fallback reason;
- AFK actions;
- late join handling.

## 11.2 `diagnostics/health_check.inc`

### Назначение
Проверка состояния системы.

### Обязательные функции
- Health_RunFullCheck()
- Health_CheckConfigs()
- Health_CheckScenarios()
- Health_CheckPlaylists()
- Health_CheckRuntime()
- Health_FormatReport()

## 11.3 `diagnostics/state_dump.inc`

### Назначение
Отладочный дамп текущего runtime-state.

Полезно для:
- Codex debugging;
- ручной диагностики;
- тестов.

---

# 12. Integrations layer

## 12.1 `integrations/game_events.inc`

### Назначение
Подписка на игровые события.

### События, которые нужно обработать
- начало карты;
- конец карты;
- round start;
- round end;
- match end, если доступно;
- player death, если влияет на сценарий;
- server empty / player count changes, если нужно.

## 12.2 `integrations/map_control.inc`

### Назначение
Изолировать все вызовы смены карты и применения cfg.

### Обязательные функции
- MapControl_ExecuteChangeLevel(map)
- MapControl_ApplyScenarioCfg(scenario)
- MapControl_ApplyBaseMode(scenario)
- MapControl_IsMapAvailable(map)

## 12.3 `integrations/timers.inc`

### Назначение
Централизованная работа с таймерами.

Нужно избегать ситуации, где десятки модулей создают несвязанные таймеры без контроля.

## 12.4 `integrations/client_hooks.inc`

### Назначение
Обработчики client connect/disconnect/putinserver и др.

## 12.5 `integrations/afk_monitor.inc`

### Назначение
Единый AFK монитор.

### Обязательные задачи
- отслеживать активность;
- помечать AFK;
- снимать AFK;
- уведомлять PlayerStateManager и ReadyManager.

---

# 13. Схема include-подключений

Codex должен избегать циклических зависимостей.

## Рекомендуемый порядок зависимостей

- `core/*` ничего не зависит от domain/presentation
- `runtime/*` зависит от core
- `config/*` зависит от core + types
- `domain/*` зависит от core + runtime + config
- `presentation/*` зависит от core + runtime + domain
- `commands/*` зависит от presentation + domain + runtime
- `diagnostics/*` зависит почти от всех, но только читает
- `integrations/*` вызывает runtime/domain/presentation

Главный принцип:
**Presentation не должен управлять Domain напрямую так, чтобы нарушать архитектуру.**

---

# 14. Порядок разработки по файлам

Ниже правильный порядок, в котором Codex должен создавать файлы.

## Этап 1. Core
Сначала создать:
- constants.inc
- enums.inc
- types.inc
- globals.inc
- helpers.inc
- forwards.inc
- debug.inc

### Что должно заработать после этапа 1
- проект компилируется как каркас;
- есть базовые типы;
- нет хаоса в enum и структурах.

## Этап 2. Runtime foundation
Создать:
- session_store.inc
- state_manager.inc
- pending_change.inc
- transition_manager.inc

### Что должно заработать после этапа 2
- можно хранить и менять runtime-state;
- state machine работает хотя бы минимально;
- есть fallback to Lobby.

## Этап 3. Config system
Создать:
- config_loader.inc
- config_core.inc
- config_scenarios.inc
- config_playlists.inc
- config_validation.inc
- scenario_registry.inc
- playlist_manager.inc

### Что должно заработать после этапа 3
- сценарии и playlists читаются из конфигов;
- можно получить сценарий по id;
- можно получить следующую карту.

## Этап 4. Player runtime logic
Создать:
- player_state_manager.inc
- afk_monitor.inc
- ready_manager.inc
- countdown_manager.inc
- client_hooks.inc

### Что должно заработать после этапа 4
- игроки учитываются системой;
- ready/check/countdown работает;
- AFK и late join можно корректно учитывать.

## Этап 5. Teams
Создать:
- team_manager.inc
- menu_team.inc

### Что должно заработать после этапа 5
- можно распределять игроков в команды;
- можно проверять валидность состава.

## Этап 6. Voting
Создать:
- vote_manager.inc
- menu_vote.inc

### Что должно заработать после этапа 6
- игроки могут голосовать;
- результат переходит в pending change.

## Этап 7. Presentation
Создать:
- status_presenter.inc
- chat_notifier.inc
- menu_play.inc
- menu_admin.inc

### Что должно заработать после этапа 7
- игрок и админ видят понятный статус;
- UI отделён от бизнес-логики.

## Этап 8. Commands
Создать:
- command_router.inc
- cmd_player.inc
- cmd_admin.inc

### Что должно заработать после этапа 8
- все команды вызывают правильные модули.

## Этап 9. Diagnostics
Создать:
- audit_logger.inc
- health_check.inc
- state_dump.inc

### Что должно заработать после этапа 9
- можно диагностировать состояние сервера.

## Этап 10. Final integrations
Создать:
- game_events.inc
- map_control.inc
- timers.inc
- завершить root serverflow.sp

### Что должно заработать после этапа 10
- плагин собирается в цельную систему;
- state machine реально управляет сервером.

---

# 15. Какие файлы считаются критическими

Если нужно сократить объём работ для MVP, нельзя выкидывать:

- serverflow.sp
- core/enums.inc
- core/types.inc
- runtime/session_store.inc
- runtime/state_manager.inc
- runtime/transition_manager.inc
- config/config_loader.inc
- config/config_scenarios.inc
- config/config_playlists.inc
- domain/scenario_registry.inc
- domain/player_state_manager.inc
- domain/ready_manager.inc
- domain/countdown_manager.inc
- domain/vote_manager.inc
- presentation/status_presenter.inc
- commands/command_router.inc
- diagnostics/health_check.inc

Без этих файлов архитектура будет неполной.

---

# 16. Что должно быть в конфигах рядом с кодом

В папке `configs/serverflow/` нужно создать минимум:

- `plugin_core.cfg`
- `scenarios.cfg`
- `playlists.cfg`
- `lang_ru.cfg`
- `lang_en.cfg`
- `examples/scenarios.example.cfg`
- `examples/playlists.example.cfg`

Codex должен не только писать код, но и подготовить **читаемые примеры конфигов**.

---

# 17. Что Codex должен реализовывать как reusable API

Каждый модуль должен иметь чистые entry points.

Примеры:

## Runtime API
- `State_TransitionTo(...)`
- `Transition_QueueScenario(...)`
- `Transition_ApplyPending()`

## Domain API
- `Scenario_Get(id)`
- `Playlist_GetNextMap(scenarioId)`
- `Ready_IsSatisfied()`
- `Team_ValidateCurrent()`
- `Vote_Create(...)`

## Presentation API
- `Status_ShowToClient(client)`
- `Notify_CountdownStarted(...)`
- `MenuPlay_Open(client)`

Это важно, чтобы потом можно было менять UI без переписывания логики.

---

# 18. Нейминг-конвенция

Codex должен соблюдать единый стиль имён.

## Для файлов
- snake_case
- понятные слова
- без “misc”, “stuff”, “temp”, “new2”

## Для функций
- префикс по модулю:
  - `State_*`
  - `Transition_*`
  - `Scenario_*`
  - `Playlist_*`
  - `Ready_*`
  - `Vote_*`
  - `Admin_*`
  - `Status_*`
  - `Health_*`

## Для enum
- UPPER_CASE_WITH_PREFIX

## Для глобальных переменных
- `g_` префикс

## Для handles/timers
- `g_h...`

---

# 19. Защита от типичных ошибок Codex

Codex часто ошибается одинаково. Ниже запреты, которые нужно прямо учитывать.

## 19.1 Запрещено
- писать огромный монолитный `mode_vote.sp`-стиль файл на всю систему;
- создавать одинаковую логику countdown в нескольких местах;
- смешивать `PrintToChat` логику и business decisions;
- напрямую вызывать смену карты из vote callbacks;
- напрямую применять cfg из menu handlers;
- хранить ready count как “ручную переменную”, не пересчитывая по player states;
- хранить scenario как набор случайных строк без валидации;
- silently fail без логов.

## 19.2 Обязательно
- каждое важное действие логировать;
- все invalid states обрабатывать guard’ами;
- для всех внешних id (scenario id, playlist id, map name) делать валидацию;
- при ошибке переходить в безопасное состояние.

---

# 20. Рекомендуемый цикл тестирования по мере реализации

После каждого этапа Codex должен поддерживать рабочую сборку.

## Минимальные тесты

### После runtime
- state transitions не ломаются;
- Lobby можно установить как safe default.

### После config
- сценарии читаются;
- invalid scenario корректно отвергается.

### После ready/countdown
- ready меняется;
- countdown стартует;
- countdown отменяется при изменении условий.

### После team manager
- open teams не стартуют без валидного состава.

### После vote manager
- vote результат не применяется мгновенно;
- создаётся pending change.

### После admin overrides
- queue override работает;
- emergency override логируется.

### После full integration
- полный цикл Lobby -> PreMatch -> Match -> PostMatch -> Transition работает без ручного вмешательства.

---

# 21. Рекомендуемый roadmap по deliverables

Codex должен отдавать работу не одной кучей, а по deliverables.

## Deliverable 1
Каркас проекта + компилирующийся plugin entry + enums/types.

## Deliverable 2
Runtime session + state manager + fallback lobby.

## Deliverable 3
Config parsing + scenario registry + playlists.

## Deliverable 4
Ready/countdown/player lifecycle.

## Deliverable 5
Teams + open squads logic.

## Deliverable 6
Vote manager + pending/confirm pipeline.

## Deliverable 7
Admin overrides + health check.

## Deliverable 8
UI/menus/status/polish.

Это лучший путь, чем писать сразу “всё и сразу”.

---

# 22. Финальная структура итогового плагина

В конце проекта должно получиться так, что:

- `serverflow.sp` только поднимает систему;
- core хранит общие типы;
- runtime управляет state machine;
- config грузит всё из файлов;
- domain решает игровую логику;
- presentation показывает людям понятный UI;
- commands только маршрутизируют;
- diagnostics умеет отлаживать;
- integrations подключают игру к системе.

Если структура проекта начинает превращаться в набор случайных вызовов из меню в cfg и обратно — это архитектурная ошибка.

---

# 23. Что считать успехом этого roadmap

Roadmap реализован успешно, если:

- любой разработчик может быстро понять, где искать нужную логику;
- добавление нового сценария не требует лезть в половину проекта;
- UI можно менять отдельно от core logic;
- state machine легко отладить;
- server lifecycle прозрачен;
- ошибки можно диагностировать через health check и logs;
- архитектура не скатывается в один большой procedural script.

---

# 24. Финальная инструкция Codex

Строй плагин строго по слоям и модулям.

Не пытайся сначала сделать “красивое меню”, а потом под него подгонять логику. Сначала создай:
- core types;
- runtime session;
- state machine;
- config-driven scenarios;
- transition pipeline.

Только после этого добавляй:
- player lifecycle;
- ready/countdown;
- teams;
- votes;
- admin overrides;
- UI.

Каждый модуль должен иметь ясную ответственность, предсказуемые публичные функции и минимальные зависимости.

Система должна оставаться читаемой, расширяемой и устойчивой к ошибкам. Это важнее, чем быстро написать первый работающий, но хрупкий вариант.

