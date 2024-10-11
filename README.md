# UDP-Cliente-Servidor
Projeto realizado por [@luiz_simon](https://github.com/luizsimon) e [@vrfrohlich](https://github.com/vrfrohlich) para a disciplina de Redes: Aplicação e Transporte

Este projeto implementa um sistema de monitoramento de tráfego TCP e UDP utilizando BPFTrace para captura e medição de bytes recebidos e enviados. Ele permite monitorar o tráfego de rede em um ou mais servidores, exibindo métricas como quantidade de dados trafegados em TCP e UDP, além de oferecer a capacidade de ajustar dinamicamente a periodicidade de envio das métricas

## Funcionalidades
- Monitoramento de tráfego TCP (bytes recebidos e enviados).
- Monitoramento de tráfego UDP (bytes recebidos e enviados).
- Ajuste dinâmico da periodicidade de envio de métricas entre clientes e servidores.
- Envio periódico de métricas dos clientes para os servidores via protocolo UDP.
- Uso de BPFTrace para capturar dados em tempo real no kernel.
- Threads para execução concorrente de múltiplas funcionalidades (monitoramento, servidor, cliente, ajuste de periodicidade).

## Requisitos
Para compilar e rodar este projeto, você precisará de:

- BPFTrace instalado para capturar os eventos de rede.
- Linux.
- GCC para compilação.
