# fakentp

Run a small ntp server with faketime to test ntp time resolution in the project.

Build:

```bash
sudo podman build -t fakentp .
```

Run with a faketime:

```bash
sudo podman run --rm -it -p 123:123/udp -e FAKETIME='@2025-03-30 00:59:00' localhost/fakentp
```
