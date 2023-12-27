
class WIFI {

public:
  WIFI(const char *ssid, const char *pw);
  bool put(StaticJsonDocument *doc);
  bool get(StaticJsonDocument *doc);
  int getStatus();

private:
  bool connect();
  const char *ssid;
  const char *pw;
  const char *server;
};
