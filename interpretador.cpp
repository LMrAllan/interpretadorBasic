#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <unordered_map>
#include <memory>

using namespace std;

// Definindo os tipos de possíveis tokens 
enum TipoToken {
    PRINT, LET, GOTO, IF, THEN, INPUT, VARIAVEL, NUMERO, STRING, OPERADOR, REM, HALT
};

// Armazenando um token
struct Token {
    TipoToken tipo;
    string valor;
};

// Estrutura do nó da árvore de sintaxe
struct NoArvore {
    string valor;
    shared_ptr<NoArvore> esquerda;
    shared_ptr<NoArvore> direita;

    NoArvore(string val) : valor(val), esquerda(nullptr), direita(nullptr) {}
};

// Tabela de variáveis para armazenar os valores das variáveis
unordered_map<string, string> tabelaVariaveis;
unordered_map<int, int> linhaParaIndice;

// Função para converter o tipo de token em string para exibição
string tipoTokenParaString(TipoToken tipo) {
    switch (tipo) {
        case PRINT: return "PRINT";
        case LET: return "LET";
        case GOTO: return "GOTO";
        case IF: return "IF";
        case THEN: return "THEN";
        case INPUT: return "INPUT";
        case VARIAVEL: return "VARIAVEL";
        case NUMERO: return "NUMERO";
        case STRING: return "STRING";
        case OPERADOR: return "OPERADOR";
        case REM: return "REM";
        case HALT: return "HALT";
    }
    // Tratamento caso não haja uns dos tokens mencionados
    cout << "Erro: tipo de token não reconhecido!" << endl;
    exit(1);
}

// Função para identificar os tokens em uma linha de código
vector<Token> analiseLexica(string linha, int linhaIndice) {
    vector<Token> tokens;
    // Expressão regular para dividir a linha em tokens
    regex reg(R"((\d+)|([A-Za-z][A-Za-z0-9]*)|(\>=|\<=|<>|\+|\-|\*|\/|=|>|<)|(\"[^\"]*\")|(\S))");
    auto palavrasInicio = sregex_iterator(linha.begin(), linha.end(), reg);
    auto palavrasFim = sregex_iterator();

    bool primeiraPalavra = true;
    for (auto i = palavrasInicio; i != palavrasFim; ++i) {
        smatch correspondencia = *i;
        string palavra = correspondencia.str();

        // Se a primeira palavra é um número, ela é tratada como um número de linha
        if (primeiraPalavra && regex_match(palavra, regex("[0-9]+"))) {
            int numeroLinha = stoi(palavra);
            linhaParaIndice[numeroLinha] = linhaIndice; // Mapeia a linha para o índice correto
            primeiraPalavra = false;
            continue;
        } else {
            primeiraPalavra = false;
        }

        // Ignora comentários na linha
        if (palavra == "REM") {
            return tokens;
        }

        // Mapeamento de palavras-chave para tipos de token
        unordered_map<string, TipoToken> palavrasChave = {
            {"PRINT", PRINT}, {"LET", LET}, {"GOTO", GOTO}, {"IF", IF}, {"THEN", THEN}, {"INPUT", INPUT}, {"REM", REM}, {"HALT", HALT}
        };

        // Verifica se a palavra é uma palavra-chave
        if (palavrasChave.find(palavra) != palavrasChave.end()) {
            tokens.push_back({palavrasChave[palavra], palavra});
        } else if (regex_match(palavra, regex("[0-9]+"))) {
            tokens.push_back({NUMERO, palavra});
        } else if (regex_match(palavra, regex("[A-Za-z][A-Za-z0-9]*"))) {
            tokens.push_back({VARIAVEL, palavra});
        } else if (palavra == "+" || palavra == "-" || palavra == "*" || palavra == "/" || palavra == "=" ||
                   palavra == ">" || palavra == "<" || palavra == ">=" || palavra == "<=" || palavra == "<>") {
            tokens.push_back({OPERADOR, palavra});
        } else if (palavra.front() == '"' && palavra.back() == '"') {
            tokens.push_back({STRING, palavra});
        } else {
            cout << "Comando desconhecido ou nao implementado: " << palavra << endl;
        }
    }

    return tokens;
}

// Função para construir a árvore de sintaxe de uma expressão
shared_ptr<NoArvore> construirArvoreSintaxe(vector<Token>& tokens, size_t& pos) {
    shared_ptr<NoArvore> no = make_shared<NoArvore>(tokens[pos].valor);
    pos++;

    // Verifica se há um operador e continua construindo a árvore
    if (pos < tokens.size() && tokens[pos].tipo == OPERADOR) {
        string operador = tokens[pos].valor;
        shared_ptr<NoArvore> operadorNo = make_shared<NoArvore>(operador);
        operadorNo->esquerda = no;
        pos++;
        operadorNo->direita = construirArvoreSintaxe(tokens, pos);
        return operadorNo;
    }

    return no;
}

// Função para avaliar a árvore de sintaxe
int avaliarArvore(shared_ptr<NoArvore> raiz) {
    // Caso base: nó folha (variável ou número)
    if (!raiz->esquerda && !raiz->direita) {
        if (tabelaVariaveis.find(raiz->valor) != tabelaVariaveis.end()) {
            return stoi(tabelaVariaveis[raiz->valor]);
        } else {
            return stoi(raiz->valor);
        }
    }

    // Avalia os nós filhos
    int esquerdaValor = avaliarArvore(raiz->esquerda);
    int direitaValor = avaliarArvore(raiz->direita);

    // Realiza a operação aritmética apropriada
    if (raiz->valor == "+") return esquerdaValor + direitaValor;
    if (raiz->valor == "-") return esquerdaValor - direitaValor;
    if (raiz->valor == "*") return esquerdaValor * direitaValor;
    if (raiz->valor == "/") {
        if (direitaValor == 0) {
            cout << "Erro: Divisao por zero." << endl;
            exit(1);
        }
        return esquerdaValor / direitaValor;
    }

    cout << "Operador desconhecido: " << raiz->valor << endl;
    exit(1);
}

// Função para avaliar a árvore de sintaxe de uma expressão condicional
bool avaliarArvoreCondicional(shared_ptr<NoArvore> raiz) {
    int esquerdaValor = avaliarArvore(raiz->esquerda);
    int direitaValor = avaliarArvore(raiz->direita);

    // Operações condicionais
    if (raiz->valor == "=") return esquerdaValor == direitaValor;
    if (raiz->valor == "<>") return esquerdaValor != direitaValor;
    if (raiz->valor == ">") return esquerdaValor > direitaValor;
    if (raiz->valor == "<") return esquerdaValor < direitaValor;
    if (raiz->valor == ">=") return esquerdaValor >= direitaValor;
    if (raiz->valor == "<=") return esquerdaValor <= direitaValor;

    cout << "Operador condicional desconhecido: " << raiz->valor << endl;
    exit(1);
}

// Função para construir a árvore de sintaxe de uma expressão condicional
shared_ptr<NoArvore> construirArvoreCondicao(vector<Token>& tokens, size_t& pos) {
    shared_ptr<NoArvore> no = make_shared<NoArvore>(tokens[pos].valor);
    pos++;

    // Verifica se há um operador e constrói a árvore da condição
    if (pos < tokens.size() && tokens[pos].tipo == OPERADOR) {
        string operador = tokens[pos].valor;
        shared_ptr<NoArvore> operadorNo = make_shared<NoArvore>(operador);
        operadorNo->esquerda = no;
        pos++;
        operadorNo->direita = make_shared<NoArvore>(tokens[pos].valor);
        pos++;
        return operadorNo;
    }

    return no;
}

// Avalia condições em comandos IF
bool avaliarCondicao(vector<Token>& tokens) {
    size_t pos = 0;
    shared_ptr<NoArvore> raiz = construirArvoreCondicao(tokens, pos);
    return avaliarArvoreCondicional(raiz);
}

// Avalia expressões aritméticas usando análise sintática
int avaliarExpressao(vector<Token>& tokens, size_t posInicio) {
    size_t pos = posInicio;
    shared_ptr<NoArvore> raiz = construirArvoreSintaxe(tokens, pos);
    return avaliarArvore(raiz);
}

// Função para executar uma linha de código BASIC
void executarLinha(vector<Token>& tokens, size_t& indiceAtual, const vector<vector<Token>>& linhasPrograma) {
    if (tokens.empty()) return;

    TipoToken comando = tokens[0].tipo;

    // Implementação dos diferentes comandos BASIC
    if (comando == PRINT) {
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i].tipo == STRING) {
                cout << tokens[i].valor.substr(1, tokens[i].valor.length() - 2) << endl;
            } else if (tokens[i].tipo == VARIAVEL) {
                if (tabelaVariaveis.find(tokens[i].valor) != tabelaVariaveis.end()) {
                    cout << tabelaVariaveis[tokens[i].valor] << endl;
                } else {
                    cout << "Variavel nao encontrada: " << tokens[i].valor << endl;
                }
            } else if (tokens[i].tipo == NUMERO) {
                cout << tokens[i].valor << endl;
            }
        }
    } else if (comando == INPUT) {
        if (tokens.size() >= 2 && tokens[1].tipo == VARIAVEL) {
            string entrada;
            getline(cin, entrada);
            tabelaVariaveis[tokens[1].valor] = entrada;
        }
    } else if (comando == LET) {
        if (tokens.size() >= 4 && tokens[1].tipo == VARIAVEL && tokens[2].tipo == OPERADOR && tokens[2].valor == "=") {
            int valor = avaliarExpressao(tokens, 3);
            tabelaVariaveis[tokens[1].valor] = to_string(valor);
        } else {
            cout << "Erro na sintaxe do comando LET." << endl;
        }
    } else if (comando == IF) {
        size_t thenPos = 0;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i].tipo == THEN) {
                thenPos = i;
                break;
            }
        }

        if (thenPos > 0) {
            // Avalia a condição entre IF e THEN
            vector<Token> condicaoTokens(tokens.begin() + 1, tokens.begin() + thenPos);
            bool condicao = avaliarCondicao(condicaoTokens);

            if (condicao) {
                // Processa o que vem após o THEN
                vector<Token> thenTokens(tokens.begin() + thenPos + 1, tokens.end());
                if (!thenTokens.empty() && thenTokens[0].tipo == GOTO && thenTokens[1].tipo == NUMERO) {
                    int linhaGOTO = stoi(thenTokens[1].valor);
                    if (linhaParaIndice.find(linhaGOTO) != linhaParaIndice.end()) {
                        indiceAtual = linhaParaIndice[linhaGOTO] - 1;
                        return;
                    } else {
                        cout << "Erro: Linha GOTO " << linhaGOTO << " não encontrada." << endl;
                    }
                } else {
                    cout << "Comando após THEN não suportado." << endl;
                }
            }
        } else {
            cout << "Erro na sintaxe do comando IF." << endl;
        }
    } else if (comando == GOTO) {
        if (tokens.size() >= 2 && tokens[1].tipo == NUMERO) {
            int linhaGOTO = stoi(tokens[1].valor);
            if (linhaParaIndice.find(linhaGOTO) != linhaParaIndice.end()) {
                indiceAtual = linhaParaIndice[linhaGOTO] - 1;
            } else {
                cout << "Erro: Linha GOTO " << linhaGOTO << " não encontrada." << endl;
            }
        }
    } else if (comando == HALT) {
        indiceAtual = linhasPrograma.size(); // Finaliza a execução
    } else if (comando == REM) {
        return;
    } else {
        cout << "Comando desconhecido ou nao implementado: " << tipoTokenParaString(comando) << endl;
    }
}

int main() {
    // Abre o arquivo que contém o código BASIC
    /* 
    Quando for testar o código setar o caminho onde está o programa junto com os txt (Operações), 
    e escolher um das quetro operacoes para ser executada.
    ------|Operacao 1 
    ------|Operacao 2
    ------|Operacao 3
    ------|Operacao 4
    */
    ifstream arquivo("C:\\Users\\MrAllan\\Desktop\\Materias Feitas\\Materias oitavo periodo\\Compiladores\\interpretadorBasic\\Operacao 1.txt");
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo" << endl;
        return 1;
    }

    string linha;
    vector<vector<Token>> linhasPrograma;

    // Lê linha por linha do programa e realiza análise léxica
    while (getline(arquivo, linha)) {
        vector<Token> tokens = analiseLexica(linha, linhasPrograma.size()); // Passa o índice da linha para mapeamento
        if (!tokens.empty()) {
            linhasPrograma.push_back(tokens);
        }
    }

    arquivo.close();

    if (linhasPrograma.empty()) {
        cout << "Nenhuma linha valida encontrada no arquivo." << endl;
        return 1;
    }

    cout << "Iniciando execucao do programa..." << endl;
    size_t indiceAtual = 0;

    // Executa cada linha do programa
    while (indiceAtual < linhasPrograma.size()) {
        executarLinha(linhasPrograma[indiceAtual], indiceAtual, linhasPrograma);
        indiceAtual++;
    }

    cout << "Execucao do programa finalizada." << endl;

    return 0;
}
