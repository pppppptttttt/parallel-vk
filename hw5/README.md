## CLI

```bash
./build/hw5 <host> <port> <path_to_hashrate_file>
```

- `host` - адрес сервера
- `port` - TCP-порт сервера
- `path_to_hashrate_file` - путь к файлу

## Тесты и примеры

```bash
bash tests/e2e/run_all.sh
```

Поднимает python сервер и запускает тестовые сценарии, см. `tests/e2e/scenarios`. Пример сценария:

```json
{
  "timeout": 10.0,
  "steps": [
    { "action": "expect", "line": "HELLO 1000000" },
    { "action": "send", "line": "TASK 342E19E13E8602F54F22DBA594FE36A97837E538 A00 A05" },
    { "action": "expect", "line": "FOUND A03" },
    { "action": "send", "line": "DONE" }
  ]
}
```

- сервер ждёт HELLO
- отправляет TASK
- ждёт FOUND A03
- отправляет DONE
