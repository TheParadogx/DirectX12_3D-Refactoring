// ============================================================
//  使用例 (コンパイル確認用; 実際の利用時は削除してよい)
// ============================================================
#ifdef SINGLETON_EXAMPLE
 
#include <iostream>
#include <string>
#include <unordered_map>
 
// --- 例1: デフォルトコンストラクタ + マクロ ---
class AppConfig : public Singleton<AppConfig> {
    SINGLETON_CLASS(AppConfig);
    SINGLETON_ACCESSOR(AppConfig);
public:
    void set(const std::string& key, const std::string& val) { data_[key] = val; }
    std::string get(const std::string& key) const {
        auto it = data_.find(key);
        return it != data_.end() ? it->second : "";
    }
private:
    std::unordered_map<std::string, std::string> data_;
};
 
// --- 例2: 引数付きコンストラクタ ---
class Connection : public Singleton<Connection> {
    SINGLETON_CLASS_CUSTOM_CTOR(Connection);
public:
    explicit Connection(std::string host, int port)
        : host_{ std::move(host) }, port_{ port } {
        std::cout << "Connection to " << host_ << ":" << port_ << "\n";
    }
    const std::string& host() const { return host_; }
    int                port() const { return port_; }
private:
    std::string host_;
    int         port_;
};
 
int main() {
    // ① デフォルトコンストラクタ版
    AppConfig::get().set("env", "production");
    std::cout << AppConfig::get().get("env") << "\n";  // production
 
    // ① 引数付きコンストラクタ版 — 初回のみ引数が使われる
    auto& c1 = Connection::getInstance("localhost", 5432);
    auto& c2 = Connection::getInstance("ignored",  9999); // 引数は無視される
    std::cout << c1.host() << ":" << c1.port() << "\n"; // localhost:5432
    std::cout << (&c1 == &c2) << "\n";                  // 1 (同一インスタンス)
 
    // ② Meyers' Singleton — hasInstance() で確認
    std::cout << Connection::hasInstance() << "\n"; // 1
 
    // ③ destroyInstance() — NO_REVIVE ポリシー
    AppConfig::destroyInstance();
    try {
        AppConfig::getInstance(); // std::logic_error を投げる
    } catch (const std::logic_error& e) {
        std::cout << "caught: " << e.what() << "\n";
    }
 
    // SINGLETON_REF マクロ
    SINGLETON_REF(Connection, conn);
    std::cout << conn.host() << "\n"; // localhost
 
    return 0;
}
 
#endif // SINGLETON_EXAMPLE