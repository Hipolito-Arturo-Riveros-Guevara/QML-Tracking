#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>

using namespace std;

// Estructura para los datos de entrada (con particle_id como long long)
struct Hit {
    int hit_id;
    double x, y, z;
    int volume_id, layer_id, module_id;
    long long particle_id;  // Cambiado a long long para int64
};

// Estructura para coordenadas cilíndricas
struct CylindricalCoords {
    double r, phi, z;
};

// Estructura para el resultado
struct TripletResult {
    double r1, phi1, z1;
    double r2, phi2, z2;
    double r3, phi3, z3;
    double theta_break, phi_break;
    int label;
};

// Función para convertir a coordenadas cilíndricas
CylindricalCoords cilindricas(double x, double y, double z, bool grados = false) {
    CylindricalCoords result;
    
    // Calcular ρ (distancia radial)
    result.r = sqrt(x*x + y*y);
    
    // Calcular φ (ángulo azimutal)
    result.phi = atan2(y, x);
    
    // Asegurar que φ esté en el rango [0, 2π)
    if (result.phi < 0) {
        result.phi += 2 * M_PI;
    }
    
    // Convertir a grados si se solicita
    if (grados) {
        result.phi = result.phi * 180.0 / M_PI;
    }
    
    result.z = z;
    return result;
}

// Función para calcular producto punto entre dos vectores 2D
double dot2D(double x1, double y1, double x2, double y2) {
    return x1*x2 + y1*y2;
}

// Función para calcular producto punto entre dos vectores 3D
double dot3D(double x1, double y1, double z1, double x2, double y2, double z2) {
    return x1*x2 + y1*y2 + z1*z2;
}

// Función para calcular norma de un vector 2D
double norm2D(double x, double y) {
    return sqrt(x*x + y*y);
}

// Función para calcular norma de un vector 3D
double norm3D(double x, double y, double z) {
    return sqrt(x*x + y*y + z*z);
}

// Función para leer el CSV con soporte para long long
vector<Hit> readCSV(const string& filename) {
    vector<Hit> data;
    ifstream file(filename);
    string line;
    
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir " << filename << endl;
        return data;
    }
    
    // Leer cabecera
    getline(file, line);
    cout << "Cabecera detectada: " << line << endl;
    
    // Leer datos
    int line_count = 0;
    int error_count = 0;
    
    while (getline(file, line)) {
        line_count++;
        
        // Limpiar la línea
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(remove(line.begin(), line.end(), '\n'), line.end());
        
        if (line.empty()) continue;
        
        stringstream ss(line);
        string token;
        Hit hit;
        vector<string> tokens;
        
        // Dividir la línea en tokens
        while (getline(ss, token, ',')) {
            // Limpiar cada token
            token.erase(remove(token.begin(), token.end(), ' '), token.end());
            token.erase(remove(token.begin(), token.end(), '\t'), token.end());
            tokens.push_back(token);
        }
        
        // Verificar número de columnas
        if (tokens.size() < 8) {
            if (error_count < 10) {  // Mostrar solo primeros 10 errores
                cerr << "Línea " << line_count << " ignorada: solo " << tokens.size() 
                     << " columnas" << endl;
            }
            error_count++;
            continue;
        }
        
        try {
            hit.hit_id = tokens[0].empty() ? 0 : stoi(tokens[0]);
            hit.x = tokens[1].empty() ? 0.0 : stod(tokens[1]);
            hit.y = tokens[2].empty() ? 0.0 : stod(tokens[2]);
            hit.z = tokens[3].empty() ? 0.0 : stod(tokens[3]);
            hit.volume_id = tokens[4].empty() ? 0 : stoi(tokens[4]);
            hit.layer_id = tokens[5].empty() ? 0 : stoi(tokens[5]);
            hit.module_id = tokens[6].empty() ? 0 : stoi(tokens[6]);
            
            // PARTICLE_ID COMO LONG LONG - esto es lo importante
            hit.particle_id = tokens[7].empty() ? 0 : stoll(tokens[7]);  // stoll para long long
            
            data.push_back(hit);
            
        } catch (const exception& e) {
            if (error_count < 10) {  // Mostrar solo primeros 10 errores
                cerr << "Error en línea " << line_count << ": " << e.what() << endl;
                cerr << "Línea problemática: " << line << endl;
                
                // Mostrar tokens individuales
                for (size_t i = 0; i < tokens.size() && i < 8; i++) {
                    cerr << "Token " << i << ": '" << tokens[i] << "'" << endl;
                }
                cerr << "---" << endl;
            }
            error_count++;
        }
        
        // Mostrar progreso
        if (line_count % 100000 == 0) {
            cout << "Líneas procesadas: " << line_count 
                 << " | Hits válidos: " << data.size() << endl;
        }
    }
    
    file.close();
    
    cout << "\n=== RESUMEN DE LECTURA ===" << endl;
    cout << "Total de líneas procesadas: " << line_count << endl;
    cout << "Hits válidos leídos: " << data.size() << endl;
    cout << "Errores: " << error_count << endl;
    
    if (data.size() > 0) {
        cout << "\nPrimeros 3 hits leídos:" << endl;
        for (int i = 0; i < min(3, (int)data.size()); i++) {
            cout << "Hit " << i << ": particle_id = " << data[i].particle_id 
                 << " (tipo: long long)" << endl;
        }
    }
    
    return data;
}

// Función para escribir el resultado
void writeCSV(const vector<TripletResult>& results, const string& filename) {
    ofstream file(filename);
    
    if (!file.is_open()) {
        cerr << "Error: No se pudo crear " << filename << endl;
        return;
    }
    
    // Escribir cabecera
    file << "r1,phi1,z1,r2,phi2,z2,r3,phi3,z3,theta_break,phi_break,label" << endl;
    
    // Escribir datos
    for (const auto& row : results) {
        file << row.r1 << "," << row.phi1 << "," << row.z1 << ","
             << row.r2 << "," << row.phi2 << "," << row.z2 << ","
             << row.r3 << "," << row.phi3 << "," << row.z3 << ","
             << row.theta_break << "," << row.phi_break << ","
             << row.label << endl;
    }
    
    file.close();
}

int main() {
    // Leer datos
    cout << "Leyendo archivo data_075.csv..." << endl;
    vector<Hit> data = readCSV("data_075.csv");
    
    if (data.empty()) {
        cerr << "Error: No se pudieron leer datos del archivo." << endl;
        return 1;
    }
    
    vector<TripletResult> results;
    int i = 0;
    
    cout << "\nIniciando procesamiento de tripletes..." << endl;
    
    // Procesar por capas
    for (int l = 1; l <= 8; l++) {  // Hasta 7 porque necesitamos l+2
        cout << "Procesando capa " << l << "..." << endl;
        
        // Filtrar hits por capa
        vector<Hit> hits_layer, hits_layer1, hits_layer2;
        
        for (const auto& hit : data) {
            if (hit.layer_id == l) hits_layer.push_back(hit);
            if (hit.layer_id == l + 1) hits_layer1.push_back(hit);
            if (hit.layer_id == l + 2) hits_layer2.push_back(hit);
        }
        
        cout << "  - Hits en capa " << l << ": " << hits_layer.size() << endl;
        cout << "  - Hits en capa " << l+1 << ": " << hits_layer1.size() << endl;
        cout << "  - Hits en capa " << l+2 << ": " << hits_layer2.size() << endl;
        
        // Procesar tripletes
        int tripletes_capa = 0;
        for (const auto& hit1 : hits_layer) {
            for (const auto& hit2 : hits_layer1) {
                // Convertir a coordenadas cilíndricas
                CylindricalCoords coords1 = cilindricas(hit1.x, hit1.y, hit1.z);
                CylindricalCoords coords2 = cilindricas(hit2.x, hit2.y, hit2.z);
                
                double r1 = coords1.r, phi1 = coords1.phi, z1 = coords1.z;
                double r2 = coords2.r, phi2 = coords2.phi, z2 = coords2.z;
                
                // Filtro del primer duplete
                double z0;
                if (hit2.x - hit1.x != 0) {
                    z0 = z1 - hit1.x * (z2 - z1) / (hit2.x - hit1.x);
                } else {
                    z0 = 1000;
                }
                
                if (abs((phi2 - phi1) / (r2 - r1)) <= 6e4 && abs(z0) <= 100) {
                    for (const auto& hit3 : hits_layer2) {
                        CylindricalCoords coords3 = cilindricas(hit3.x, hit3.y, hit3.z);
                        double r3 = coords3.r, phi3 = coords3.phi, z3 = coords3.z;
                        
                        // Filtro del segundo duplete
                        double z00;
                        if (hit3.x - hit2.x != 0) {
                            z00 = z2 - hit2.x * (z3 - z2) / (hit3.x - hit2.x);
                        } else {
                            z00 = 1000;
                        }
                        
                        if (abs((phi3 - phi2) / (r3 - r2)) <= 6e4 && abs(z00) <= 100) {
                            // Calcular parámetros de tripletes
                            
                            // theta_break
                            double v12_x = hit2.x - hit1.x;
                            double v12_y = hit2.y - hit1.y;
                            double v12_z = hit2.z - hit1.z;
                            
                            double v23_x = hit3.x - hit2.x;
                            double v23_y = hit3.y - hit2.y;
                            double v23_z = hit3.z - hit2.z;
                            
                            double dot_v = dot3D(v12_x, v12_y, v12_z, v23_x, v23_y, v23_z);
                            double norm_v12 = norm3D(v12_x, v12_y, v12_z);
                            double norm_v23 = norm3D(v23_x, v23_y, v23_z);
                            
                            double cos_theta = dot_v / (norm_v12 * norm_v23);
                            cos_theta = max(-1.0, min(1.0, cos_theta));
                            double theta_break = acos(cos_theta);
                            
                            // phi_break
                            double f12_z = z2 - z1;
                            double f12_r = r2 - r1;
                            
                            double f23_z = z3 - z2;
                            double f23_r = r3 - r2;
                            
                            double dot_f = dot2D(f12_z, f12_r, f23_z, f23_r);
                            double norm_f12 = norm2D(f12_z, f12_r);
                            double norm_f23 = norm2D(f23_z, f23_r);
                            
                            double cos_phi = dot_f / (norm_f12 * norm_f23);
                            cos_phi = max(-1.0, min(1.0, cos_phi));
                            double phi_break = acos(cos_phi);
                            
                            if (theta_break <= 0.05 && phi_break <= 0.05) {
                                int label = 0;
                                if (hit1.particle_id == hit2.particle_id && 
                                    hit2.particle_id == hit3.particle_id) {
                                    label = 1;
                                }
                                
                                TripletResult result;
                                result.r1 = r1;
                                result.phi1 = phi1;
                                result.z1 = z1;
                                result.r2 = r2;
                                result.phi2 = phi2;
                                result.z2 = z2;
                                result.r3 = r3;
                                result.phi3 = phi3;
                                result.z3 = z3;
                                result.theta_break = theta_break;
                                result.phi_break = phi_break;
                                result.label = label;
                                
                                results.push_back(result);
                                i++;
                                tripletes_capa++;
                                
                                if (i % 1000 == 0) {
                                    cout << "  Progreso - Capa: " << l 
                                         << " | Tripletes totales: " << i << endl;
                                }
                            }
                        }
                    }
                }
            }
        }
        cout << "  - Tripletes encontrados en capa " << l << ": " << tripletes_capa << endl;
    }
    
    // Escribir resultados
    cout << "\nEscribiendo resultados..." << endl;
    writeCSV(results, "resultado.csv");
    cout << "=== PROCESAMIENTO COMPLETADO ===" << endl;
    cout << "Total de tripletes: " << i << endl;
    cout << "Resultados guardados en resultado.csv" << endl;
    
    return 0;
}