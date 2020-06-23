# Projekt NS-3

## Wytyczne

Filtrowanie powinno być możliwe w doniesieniu do pól:

* Cellid, IMSI, RNTI (dla wszystkich plików z wyjątkiem pliku *.tr)
* dla pliku *.tr filtrowanie powinno być możliwe w odniesieniu do pól:
    * typ zdarzenia (podany w pierwszej kolumnie): + lub -
    * adres nadawcy , adres odbiorcy. Tu jest przykład fragmentu opisującego adres nadawcy i adres odbiorcy:
1.0.0.2 > 7.0.0.2
    * typ protokołu: GtpuHeader, SeqTsHeader
    * zliczanie wolumenu przesłanego ruchu i obliczanie  przepływności strumieni (wolumen podzielony przez przedział czasu) - proszę dokonać wizualizacji
danych ilościowych na wykresach z pomocą biblioteki np. matlibplot.



## Parser
Parser wykonany w jezyku python3 analizuje pliki wyjsciowe symulacji przeprowadzonej w symulatorze ns-3 korzystajac
 ze scenariusza lena-deactivate-bearer.
Wymagane bibioteki znajduja sie w pliku requiments.txt

parser uruchamiany przez
python3 ./parser.py <log_file> <params>

parametrami moga byc filtry z wartosciami np

python3 ./parser.py log.tr -type +
(type, protocol, time) dla logow tr
(CELLID, IMSI, RNTI) dla txt

jednym z parametrow jest -plot umozliwiajacy narysowanie wykresu obciazenia lacza z pliku log.tr
dodatkowo -payloadonly rysuje tylko wartosci payload bez wielkosci ramek


W katalogu dane znajduja sie przykladowe pliki wyjsciowe symulatora, pozwalaja one na testy parsera.


## Praca z symulatorem

* kompilacja

cd ~/ns-3-dev
./waf build

* uruchomienie plus kopia logow

cd ~/ns-3-dev
./waf shell
cd ~/ns-3-dev/testrun
../build/src/lte/examples/ns3-dev-lena-deactivate-bearer-debug

rm /home/kn/Desktop/projekt/dane/*

rm ~/ns-3-dev/testrun/*

cp /home/kn/ns-3-dev/testrun/* /home/kn/Desktop/projekt/dane/