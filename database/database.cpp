#include "database.h"
#include <iostream>
#include <sstream>

Database::Database(const std::string& path) : dbPath(path), db(nullptr) {}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Database::inicializar() {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc) {
        std::cerr << "Error al abrir DB: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS mensajes ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "remitente TEXT NOT NULL,"
                      "destinatario TEXT NOT NULL,"
                      "contenido TEXT NOT NULL,"
                      "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Error SQL: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    std::cout << "✅ Base de datos inicializada" << std::endl;
    return true;
}

bool Database::guardarMensaje(const std::string& remitente, 
                              const std::string& destinatario,
                              const std::string& contenido) {
    std::string sql = "INSERT INTO mensajes (remitente, destinatario, contenido) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, remitente.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, destinatario.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, contenido.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE);
}

std::vector<Mensaje> Database::obtenerHistorial() {
    std::vector<Mensaje> mensajes;
    const char* sql = "SELECT id, remitente, destinatario, contenido, timestamp FROM mensajes ORDER BY timestamp;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return mensajes;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Mensaje m;
        m.id = sqlite3_column_int(stmt, 0);
        m.remitente = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        m.destinatario = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        m.contenido = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        m.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        mensajes.push_back(m);
    }
    
    sqlite3_finalize(stmt);
    return mensajes;
}

std::vector<Mensaje> Database::obtenerMensajesPorDispositivo(const std::string& dispositivo) {
    std::vector<Mensaje> mensajes;
    std::string sql = "SELECT id, remitente, destinatario, contenido, timestamp FROM mensajes "
                      "WHERE remitente = ? OR destinatario = ? ORDER BY timestamp;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return mensajes;
    }
    
    sqlite3_bind_text(stmt, 1, dispositivo.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, dispositivo.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Mensaje m;
        m.id = sqlite3_column_int(stmt, 0);
        m.remitente = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        m.destinatario = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        m.contenido = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        m.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        mensajes.push_back(m);
    }
    
    sqlite3_finalize(stmt);
    return mensajes;
}