#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>

struct Mensaje
{
    int id;
    std::string remitente;
    std::string destinatario;
    std::string contenido;
    std::string timestamp;
};

class Database
{
private:
    sqlite3 *db;
    std::string dbPath;

public:
    Database(const std::string &path);
    ~Database();

    bool inicializar();
    bool guardarMensaje(const std::string &remitente,
                        const std::string &destinatario,
                        const std::string &contenido);
    std::vector<Mensaje> obtenerHistorial();
    std::vector<Mensaje> obtenerMensajesPorDispositivo(const std::string &dispositivo);
};

#endif