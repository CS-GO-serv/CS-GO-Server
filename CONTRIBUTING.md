# Contributing

## Scope PR

Активные PR должны содержать изменения только в утверждённом scope:
- `csgo/addons/sourcemod/scripting/serverflow/**`
- `csgo/addons/sourcemod/scripting/serverflow.sp`
- `csgo/addons/sourcemod/configs/serverflow/**`
- `docs/**`
- скрипты запуска/сборки: `build_serverflow.bat`, `start.bat`, `start_classic.bat`, `start_dz.bat`
- необходимые текстовые артефакты (`Log/**`, `serverflow.smx.b64.txt`)

Нужно убирать из PR-диффа массовые изменения вне scope, особенно:
- `csgo/addons/sourcemod/scripting/base*`
- `csgo/addons/sourcemod/scripting/fun*`
- `csgo/addons/sourcemod/scripting/include/*`
- `csgo/addons/sourcemod/scripting/testsuite/*`

Рекомендуемая очистка диффа перед коммитом:

```bash
git restore --staged --worktree \
  csgo/addons/sourcemod/scripting/base* \
  csgo/addons/sourcemod/scripting/fun* \
  csgo/addons/sourcemod/scripting/include \
  csgo/addons/sourcemod/scripting/testsuite
```
