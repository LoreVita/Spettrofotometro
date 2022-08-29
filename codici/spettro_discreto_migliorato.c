void spettro_discreto_migliorato()
{
	//INSERIRE nome del file di testo contenente le misure
	const char *NOME_FILE = "azoto.txt";
	fstream filein(NOME_FILE,ios::in);
	
	//Definizione variabili per la lettura del file 
    string line, lambda, intensity;  
    int n = 0;
	int i; 
	const int nbins = 1280;
    double x[nbins], y[nbins]; 
    string dot = ".";
	
	//Lettura dei dati da utilizzare 
	if (filein.is_open()){
		for (i=0; i<16; i++) getline (filein,line); // Legge 16 righe di intestazione
		//Ciclo while per leggere i dati utili dal file
        while (filein >> lambda >> intensity){
            lambda.replace(3,1,dot); //sostituisce virgola con punto
            intensity.replace(3,1,dot); //sostituisce virgola con punto
            x[n] = atof(lambda.c_str()); //conversione string --> char --> double
            y[n] = atof(intensity.c_str()); //conversione string --> char --> double
            n++;
 		    //cout << x[n] << "-" << y[n] << endl;
        }//end ciclo while
     }//end if
	 
	 //Istogramma spettro 
	 TH1F *h = new TH1F("h", " ; #lambda (nm); intensity (%)", nbins, 0, nbins);
	 for (i=0; i<n; i++) h->SetBinContent(x[i], (int) y[i]);
	 h->SetLineColor(kBlack);
	 h->SetStats(0);
	 //Range da mostrare
	 h->GetYaxis()->SetRangeUser(0,100);  
	 h->GetXaxis()->SetRangeUser(400,900);
	 //Titoli assi centrati
     h->GetXaxis()->SetTitle("Wavelenght (nm)");
	 h->GetXaxis()->CenterTitle(true);
     h->GetYaxis()->SetTitle("Intensity (%)");
	 h->GetYaxis()->CenterTitle(true);
	 //Titolo del grafico
	 h->SetTitle("TITOLO"); //<----INSERIRE
	 
 	 //Prima finestra di lavoro
 	 TCanvas *c1 = new TCanvas("c1","",800,600);
	 h->Draw();
	 
	 //Seconda finestra di lavoro
	 TCanvas *c2 = new TCanvas("c2","",800,600);
	 
	 //Operazione di fit globale
	 
	 //Definizione variabili
     double PeakPositionX[100], PeakPositionY[100];
     int nfound,bin;
     double a;
     double source[nbins], dest[nbins];
	 double mean[100], sigma[100], Errmean[100], Errsigma[100];
	 
	 //Valori da MODIFICARE a discrezione dello sperimentatore a seconda del caso 
	 double sigma_peak = 1; //sigma of searched peaks
	 double threshold = 1;  //threshold value in % for selected peaks (peaks with amplitude less than threshold*highest_peak/100 are ignored)
	 float offset = 3; //larghezza approssimativa del picco che si fitta
	 
	 //Classe che usa la funzione SearchHighRes per scovare i picchi
	 //https://root.cern/doc/master/classTSpectrum.html
     TSpectrum *s = new TSpectrum();
	 
	 //Instogramma su cui si sovrappone il fit
     TH1F *hfit = (TH1F*) h->Clone("hfit");
	 hfit->SetTitle("LOL"); //<----INSERIRE
	 
	 //Ricerca dei picchi
	 for (i=0; i<nbins; i++) source[i] = hfit->GetBinContent(i+1);
     nfound = s->SearchHighRes(source, dest, nbins, sigma_peak, threshold, kTRUE, 3, kTRUE, 3);
	 
	 //Determinazione coordiante dei picchi
     double *xpeaks = s->GetPositionX();
     for (i=0; i<nfound; i++) {
         a=xpeaks[i];
         bin = 1 + int(a + 0.5);
         PeakPositionX[i] = h->GetBinCenter(bin);
         PeakPositionY[i] = h->GetBinContent(bin);
     }
	 
	 //Marcatore picchi
     TPolyMarker * pm = (TPolyMarker*)hfit->GetListOfFunctions()->FindObject("TPolyMarker");
     if (pm) {
        hfit->GetListOfFunctions()->Remove(pm);
        delete pm;
     }
     pm = new TPolyMarker(nfound, PeakPositionX, PeakPositionY);
     hfit->GetListOfFunctions()->Add(pm);
     pm->SetMarkerStyle(23);
     pm->SetMarkerColor(kRed);
     pm->SetMarkerSize(1.3);
	 
	 //Ciclo for per fittare ogni picco  
	 for (i=0; i<nfound; i++){
		 //Fit di tipo gaussiana
		 TF1 *ffit = new TF1("ffit","gaus");
		 //Si danno alcuni valori indicativi al fit
		 ffit->SetParameters(1,PeakPositionX[i]); //centroide
		 ffit->SetParameters(0,PeakPositionY[i]); //altezza
		 //Fit mediante metodo massima veromiglianza	 
		 hfit->Fit("ffit","+QL","", PeakPositionX[i]-offset, PeakPositionX[i]+offset); //range del fit che si può far variare come offset dal valore del centroide
		 mean[i] = ffit->GetParameter(1);
		 Errmean[i] = ffit->GetParError(1);
		 sigma[i] = ffit->GetParameter(2);
		 Errsigma[i] = ffit->GetParError(2);
		 ffit->SetLineColor(kBlack);
	 }
	 
     hfit->Draw("same");
	 
	 //Stampa dei risultati
	 for (i=0; i<nfound; i++){
		 cout << "\nPicco n." << i+1 << endl;
		 cout << "Il centroide del picco è: " << mean[i] << " +/- " << Errmean[i] << endl;
		 cout << "La sigma del picco vale:  " << sigma[i] << " +/- " << Errsigma[i] << endl;
	 }
	 
	 //Ulteriore instogramma carino
	 TH1F *hp = (TH1F*) h->Clone("hp");
	 hp->SetTitle("LOL n.3"); //<----INSERIRE
	 //Riempimento instogramma con i valori "puliti" per evidenziare meglio le coordinate dei picchi
	 float correction = 1; //termine per aggiustare la resa grafita del fit (rosso)
	 for(i=0; i<nbins; i++) hp->SetBinContent(i+1,dest[i]*correction);
     hp->SetLineColor(kRed);
	 
     hp->GetListOfFunctions()->Add(pm);
     pm->SetMarkerStyle(23);
     pm->SetMarkerColor(kRed);
     pm->SetMarkerSize(1.3);
	 
	 //Terza finestra di lavoro
	 TCanvas *c3 = new TCanvas("c3","",800,600);
     hp->Draw();
	 h->Draw("SAME");
}