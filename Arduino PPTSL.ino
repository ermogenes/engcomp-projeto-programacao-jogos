#include <LiquidCrystal.h>

// Configuração de Hardware

const int PIN_BTN_PEDRA = 6;
const int PIN_BTN_PAPEL = 7;
const int PIN_BTN_TESOURA = 8;
const int PIN_BTN_SPOCK = 9;
const int PIN_BTN_LAGARTO = 10;
const int NENHUM_BOTAO = 0;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Configurações do jogo

const int RND_SEMENTE = 1400882;

const int PONTOS_NECESSARIOS = 3;

const int TEMPO_JOGADA_CPU = 500;
const int TEMPO_LEITURA = 2000;

// Constantes de apoio

const int PLAYER = 0;
const int CPU = 1;
const int EMPATE = 0;

String NOME_JOGADOR[] = { "JOGADOR", "IA" };
String NOME_JOGADOR_RESUMIDO[] = { "JOG", "IA" };

typedef enum {
	ligado = 0,
	nova_rodada = 1,
	aguardando_player = 2,
	fim_rodada = 3,
	fim_partida = 4,
	finalizado = 5
} Estados;

// Regras

int resultado[5][5] = {
	{ EMPATE,			PIN_BTN_PAPEL,		PIN_BTN_PEDRA,		PIN_BTN_SPOCK,		PIN_BTN_PEDRA},
	{ PIN_BTN_PAPEL,	EMPATE,				PIN_BTN_TESOURA,	PIN_BTN_PAPEL,		PIN_BTN_LAGARTO},
	{ PIN_BTN_PEDRA,	PIN_BTN_TESOURA,	EMPATE,				PIN_BTN_SPOCK,		PIN_BTN_TESOURA},
	{ PIN_BTN_SPOCK,	PIN_BTN_PAPEL,		PIN_BTN_SPOCK,		EMPATE,				PIN_BTN_LAGARTO},
	{ PIN_BTN_PEDRA,	PIN_BTN_LAGARTO,	PIN_BTN_TESOURA,	PIN_BTN_LAGARTO,	EMPATE}
};

// Controle do jogo

int pontos[] = {CPU, PLAYER};
int rodada_atual;
int jogada_player;
int jogada_cpu;
Estados estado;

// Início da Partida

void setup()
{
	setPinosBotoes();
	randomSeed(RND_SEMENTE);
	lcd.begin(16, 2);
	estado = ligado;
}

// O jogo
void loop()
{
	switch (estado)
	{
		case ligado:
			iniciaPartida();
			estado = nova_rodada;
			break;

		case nova_rodada:
			iniciaRodada();
			estado = aguardando_player;
			break;

		case aguardando_player:
			jogada_player = avaliaBotoes();
			if (jogada_player != NENHUM_BOTAO)
			{
				jogada_cpu = avaliaJogadaCPU();
				efetivaJogada(jogada_player, jogada_cpu);
				estado = fim_rodada;
			}
			break;

		case fim_rodada:
			finalizaRodada();
			if (pontos[PLAYER] >= PONTOS_NECESSARIOS || pontos[CPU] >= PONTOS_NECESSARIOS)
			{
				estado = fim_partida;
			}
			else
			{   
				estado = nova_rodada;
			}
			break;

		case fim_partida:
			finalizaPartida();
			estado = finalizado;
			break;

		case finalizado:
			break;      
	}
}

// *************************** Configurações ************************************

void setPinosBotoes()
{
	pinMode(PIN_BTN_PEDRA, INPUT_PULLUP);
	pinMode(PIN_BTN_PAPEL, INPUT_PULLUP);
	pinMode(PIN_BTN_TESOURA, INPUT_PULLUP);
	pinMode(PIN_BTN_SPOCK, INPUT_PULLUP);
	pinMode(PIN_BTN_LAGARTO, INPUT_PULLUP);
}

// *************************** Jogo ************************************

void iniciaPartida()
{
	pontos[CPU] = 0;
	pontos[PLAYER] = 0;
	rodada_atual = 0;

	uiInicioPartida();
}

void iniciaRodada()
{
	rodada_atual++;
	jogada_player = 0;

	uiInicioRodada(rodada_atual);
	uiAguardaJogadaPlayer();
}

int avaliaBotoes()
{
	if (digitalRead(PIN_BTN_PEDRA) 	 == LOW) return PIN_BTN_PEDRA;
	if (digitalRead(PIN_BTN_PAPEL)	 == LOW) return PIN_BTN_PAPEL;
	if (digitalRead(PIN_BTN_TESOURA) == LOW) return PIN_BTN_TESOURA;
	if (digitalRead(PIN_BTN_SPOCK) 	 == LOW) return PIN_BTN_SPOCK;
	if (digitalRead(PIN_BTN_LAGARTO) == LOW) return PIN_BTN_LAGARTO;
	return NENHUM_BOTAO;
}

int avaliaJogadaCPU()
{
	uiAguardaJogadaCPU();
	return random(PIN_BTN_PEDRA, PIN_BTN_LAGARTO);
}

int avaliaVencedorPartida()
{
	return pontos[PLAYER] <= pontos[CPU];
}

void efetivaJogada(int jogada_player, int jogada_cpu)
{
	if (jogada_player == jogada_cpu)
	{
		uiJogadaEmpatada();
	}
	else
	{
		int vencedor = avaliaVencedorMao(jogada_player, jogada_cpu);
		int temp, opcao_vencedor, opcao_perdedor;
		if (vencedor == CPU)
		{
			opcao_vencedor = jogada_cpu;
			opcao_perdedor = jogada_player;
		}    
		else
		{
			opcao_vencedor = jogada_player;
			opcao_perdedor = jogada_cpu;
		}
		uiJogadaComVencedor(opcao_vencedor, opcao_perdedor, verboConfronto(opcao_vencedor, opcao_perdedor), vencedor);
		pontos[vencedor]++;
	}
}

void finalizaRodada()
{
	uiPlacar();
}

void finalizaPartida()
{
	uiVitoria(avaliaVencedorPartida());
}

int avaliaVencedorMao(int jogada_player, int jogada_cpu)
{
	int ganhou = resultado[jogada_player - PIN_BTN_PEDRA][jogada_cpu - PIN_BTN_PEDRA];
	return ganhou == jogada_cpu;
}

// **************************** UI ************************************

String textoOpcao(int opcao)
{
	switch (opcao)
	{
		case PIN_BTN_PEDRA: 	return "PEDRA";
		case PIN_BTN_PAPEL: 	return "PAPEL";
		case PIN_BTN_TESOURA: 	return "TESOURA";
		case PIN_BTN_SPOCK: 	return "SPOCK";
		case PIN_BTN_LAGARTO: 	return "LAGARTO";
	}
	return "";
}

String verboConfronto(int opcao1, int opcao2)
{
// TESOURA corta PAPEL cobre PEDRA esmaga LAGARTO envenena SPOCK quebra TESOURA decapita LAGARTO engole PAPEL desmente SPOCK vaporiza PEDRA esmaga TESOURA
	if (opcao1 == PIN_BTN_TESOURA && opcao2 == PIN_BTN_PAPEL) 	return "corta";
	if (opcao1 == PIN_BTN_PAPEL   && opcao2 == PIN_BTN_PEDRA) 	return "cobre";
	if (opcao1 == PIN_BTN_PEDRA   && opcao2 == PIN_BTN_LAGARTO) return "esmaga";
	if (opcao1 == PIN_BTN_LAGARTO && opcao2 == PIN_BTN_SPOCK) 	return "envenena";
	if (opcao1 == PIN_BTN_SPOCK   && opcao2 == PIN_BTN_TESOURA) return "quebra";
	if (opcao1 == PIN_BTN_TESOURA && opcao2 == PIN_BTN_LAGARTO) return "decapita";
	if (opcao1 == PIN_BTN_LAGARTO && opcao2 == PIN_BTN_PAPEL) 	return "engole";
	if (opcao1 == PIN_BTN_PAPEL   && opcao2 == PIN_BTN_SPOCK) 	return "desmente";
	if (opcao1 == PIN_BTN_SPOCK   && opcao2 == PIN_BTN_PEDRA) 	return "vaporiza";
	if (opcao1 == PIN_BTN_PEDRA   && opcao2 == PIN_BTN_TESOURA) return "esmaga";
}

void exibeUI(String linha1, String linha2)
{
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print(linha1);
	lcd.setCursor(0, 1); lcd.print(linha2); 
}

void uiInicioPartida()
{
	exibeUI(" Arduino PPTSL ",
			" Iniciando...  ");
	delay(TEMPO_LEITURA);
}

void uiInicioRodada(int rodada)
{
	exibeUI(" Rodada numero ",
			"       " + String(rodada));
	delay(TEMPO_LEITURA);
}

void uiAguardaJogadaPlayer()
{
	exibeUI("  Escolha sua  ",
			"      mao:     ");
}

void uiAguardaJogadaCPU()
{
	exibeUI(" Hum...        ",
			"  Minha vez... ");
	delay(TEMPO_JOGADA_CPU);

	exibeUI("  Ok, escolhi. ",
			"Vamos conferir!");
	delay(TEMPO_LEITURA);
}

void uiJogadaComVencedor(int opcao_vencedor, int opcao_perdedor, String verboConfronto, int vencedor)
{
	exibeUI(textoOpcao(opcao_vencedor),
			verboConfronto + " " + textoOpcao(opcao_perdedor));
	delay(TEMPO_LEITURA);

	exibeUI("   Vencedor:   ",
			"     " + NOME_JOGADOR[vencedor]);
	delay(TEMPO_LEITURA);
}

void uiJogadaEmpatada()
{
	exibeUI(" Nao acredito! ",
			"   Empatamos!  ");
	delay(TEMPO_LEITURA);
}

void uiVitoria(int vencedor)
{
	String verbo = vencedor == PLAYER ? "venceu" : "perdeu";
	String icone = vencedor == PLAYER ? ":)" : ":(";

	exibeUI(" Voce " + verbo,
			" a partida " + icone);
	delay(TEMPO_LEITURA);
}

void uiPlacar()
{
	exibeUI(" Placar:        ",
			NOME_JOGADOR_RESUMIDO[PLAYER] + ": " + pontos[PLAYER] + " x " + pontos[CPU] + " :" + NOME_JOGADOR_RESUMIDO[CPU]);
	delay(TEMPO_LEITURA);
}
