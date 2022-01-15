/* 
 * File:   main.cpp
 * Author: Lorena 11721BCC019
 */

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>


class MeuArquivo {
public:
    struct cabecalho { 
        int quantidade; 
        int disponivel; 
    } cabecalho;

    // construtor: abre arquivo. Essa aplicacao deveria ler o arquivo se existente ou criar um novo.
    // Entretando recriaremos o arquivo a cada execucao ("w+).
    MeuArquivo() {
        fd = fopen("meuarquivo.dat","w+");
        cabecalho.quantidade = 0;
        cabecalho.disponivel = -1;
        atualizaCabecalho();
    }

    // Destrutor: fecha arquivo
    ~MeuArquivo() {
        fclose(fd);
    }

    // Insere uma nova palavra, consulta se há espaco disponível ou se deve inserir no final
    void inserePalavra(char *palavra) {
        this->substituiBarraNporBarraZero(palavra); // funcao auxiliar substitui terminador por \0
        cabecalho.quantidade++;
        char len = strlen(palavra);
        // nao ha espaco disponivel, inserir no final
        if (cabecalho.disponivel == -1) {
            fseek(fd,0,SEEK_END);
            fwrite(&len,sizeof(char),1,fd);
            fprintf(fd,"%s",palavra);
        }
        // tentar inserir na lista de disponiveis
        else {
            fseek(fd, SEEK_SET, 0);
            while(!feof(fd)){
                if(!strcmp(palavra,"*")) // compara o nome da musica com o nome da musica desejada
                    if (strlen(palavra) <= len) {
                        len = strlen(palavra);
                        fwrite(&len,sizeof(char),1,fd);
                        fprintf(fd,"%s",palavra);
                    }
            }    
        }
        atualizaCabecalho();
    }

    //! Marca registro como removido, atualiza lista de disponíveis, incluindo o cabecalho
    void removePalavra(int offset) {
        if(cabecalho.quantidade == 0)
            return;  

        cabecalho.quantidade--;    

        fseek(fd,offset,SEEK_SET);

        char *palavra = new char[250];
        char len;
        fread(&len,sizeof(char),1,fd);

        palavra[1] = '*';

        atualizaCabecalho();
    }

    char *retornaPalavra(int offset) {
        fseek(fd,offset,SEEK_SET);
        char *palavra = new char[250];
        char len;
        fread(&len,sizeof(char),1,fd);
        // ler palavra
        for (int i = 0; i < len; i++) {
            palavra[i] = fgetc(fd);
            palavra[i+1] = '\0';
        }
        return palavra;
    }

    bool alteraPalavra(char *palavra, int offset) {
        this->substituiBarraNporBarraZero(palavra); // funcao auxiliar substitui terminador por \0

        fseek(fd,offset,SEEK_SET);
        char len;
        fread(&len,sizeof(char),1,fd);

        if (strlen(palavra) <= len) {
            fseek(fd,offset,SEEK_SET);
            len = strlen(palavra);
            fwrite(&len,sizeof(char),1,fd);
            fprintf(fd,"%s",palavra);
            return true;
        }
        return false;
    }

    // BuscaPalavra: retorno é o offset para o registro
    // Nao deve considerar registro removido
    int buscaPalavra(char *palavra) {
        this->substituiBarraNporBarraZero(palavra); // funcao auxiliar substitui terminador por \0

        // comecar do inicio do arquivo
        fseek(fd,0,SEEK_SET);

        // ler cabecalho
        fread(&cabecalho,sizeof(cabecalho),1,fd);

        int offset = sizeof(cabecalho);
        char palavraregistro[250];
        // para todos os registro (cabecalho.quantidade)
        for (int i = 0; i < cabecalho.quantidade; i++) {
            // ler tamanho da palavra
            char len;
            fread(&len,sizeof(char),1,fd);
            // ler palavra
            for (int i = 0; i < len; i++) {
                palavraregistro[i] = fgetc(fd);
                palavraregistro[i+1] = '\0';
            }
            // comparar palavraregistro com palavra buscada
            if (strcmp(palavraregistro,palavra) == 0) {
                return offset;
            }
            offset += sizeof(char) * (len + 1);
        }

        // retornar -1 caso nao encontrar
        return -1;
    }

private:
    // descritor do arquivo é privado, apenas métodos da classe podem acessa-lo
    FILE *fd;

    void atualizaCabecalho() {
        // pegar offset
        fpos_t offset;
        fgetpos(fd,&offset);

        // fseek para o inicio
        fseek(fd,0,SEEK_SET);

        // gravar cabecalho
        fwrite(&cabecalho,sizeof(cabecalho),1,fd);

        // fseek para offset original
        fsetpos(fd,&offset);
    }

    // funcao auxiliar substitui terminador por \0
    void substituiBarraNporBarraZero(char *str) {
        int tam = strlen(str); for (int i = 0; i < tam; i++) if (str[i] == '\n') str[i] = '\0';
    }


};

int main(int argc, char** argv) {

    setlocale(LC_ALL,""); 

    // abrindo arquivo aurelio.txt
    FILE *f = fopen("aurelio.txt","rt");

    // se não abriu
    if (f == NULL) {
        printf("Erro ao abrir arquivo.\n\n");
        return 0;
    }

    char *palavra = new char[50];

    // criando arquivo de dados
    MeuArquivo *arquivo = new MeuArquivo();
    while (!feof(f)) {
        fgets(palavra,50,f);
        arquivo->inserePalavra(palavra);
    }

    // fechar arquivo aurelio.txt

    fclose(f);

    printf("Arquivo criado.\n\n");

    char opcao;
    do {
        printf("\n\n1-Insere\n2-Remove\n3-Busca\n4-Altera\n5-Sair\nOpcao:");
        opcao = getchar();
        if (opcao == '1') {
            printf("Palavra: ");
            scanf("%s",palavra);
            arquivo->inserePalavra(palavra);
        }
        else if (opcao == '2') {
            printf("Palavra: ");
            scanf("%s",palavra);
            int offset = arquivo->buscaPalavra(palavra);
            if (offset >= 0) {
                arquivo->removePalavra(offset);
                printf("Removido.\n\n");
            }
        }
        else if (opcao == '3') {
            printf("Palavra: ");
            scanf("%s",palavra);
            int offset = arquivo->buscaPalavra(palavra);
            if (offset >= 0) {
                char *tmp = arquivo->retornaPalavra(offset);
                printf("Encontrou %s na posição %d\n\n",tmp,offset);
                delete[] tmp;
            }
            else
                printf("Não encontrou %s\n\n",palavra);
        }
        else if (opcao == '4') {
            printf("Palavra: ");
            scanf("%s",palavra);
            int offset = arquivo->buscaPalavra(palavra);
            if (offset >= 0) {
                printf("Nova palavra: ");
                scanf("%s",palavra);
                bool alterado = arquivo->alteraPalavra(palavra,offset);
                if (alterado)
                    printf("Encontrou %s na posição %d.\nPalavra alterada.\n\n",palavra,offset);
                else
                    printf("Encontrou %s na posição %d.\nPalavra nao alterada por ser maior que palavra existente.\n\n",palavra,offset);
            }
            else {
                printf("Não encontrou %s\n\n",palavra);
            }
        }
        if (opcao != '5') opcao = getchar();
    } while (opcao != '5');

    printf("\n\nAte mais!\n\n");

    delete arquivo;

    return (EXIT_SUCCESS);
}
