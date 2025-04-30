    #include <iostream>
    #include <common.h>
    #include <cstdlib>
    #include <string.h>
    #include <random>
    #include "utils/math_utils.h"

    int img_depth;
    int seed=0;
    cv::Mat img;
    cv::Mat noisy_img;


    /**
    * @brief Ajout de bruit poivre et sel
    *
    * @param img Image à modifier                                                                                  //TODO
    * @param max_pepper Pourcentage maximum de poivre
    * @param bright bright = 0 : Neutre; bright > 0 : Éclaircit; bright < 0 : Assombrit
    *
    */


    void add_salt_pepper_noise(cv::Mat &img, cv::RNG rng, float max_pepper, float max_salt)      //chevauchements
    {   
        int amount1=img.rows*img.cols*max_pepper/100;   // /100 pour passer un pourcentage entier en paramètre
        int amount2=img.rows*img.cols*max_salt/100;
        for(int counter=0; counter<amount1; ++counter)
        {
            img.at<cv::Vec3b>(rng.uniform(0,img.rows), rng.uniform(0, img.cols)) = cv::Vec3b(0, 0, 0);
        }
        for (int counter=0; counter<amount2; ++counter){
            img.at<cv::Vec3b>(rng.uniform(0,img.rows), rng.uniform(0,img.cols)) = cv::Vec3b(255, 255, 255);
        }
    }


    /**
    * @brief Ajout de bruit gaussien
    *
    * @param img Image à modifier
    * @param contrast contrast = 1 : Pas de changement; contrast > 1 : Augmente le contraste; 0 < contrast < 1 : Réduit le contraste
    * @param bright bright = 0 : Neutre; bright > 0 : Éclaircit; bright < 0 : Assombrit
    *
    */

    void add_gaussian_noise(cv::Mat &img, cv::RNG rng, int dispersion, int offset) {
        cv::Mat noise = cv::Mat::zeros(img.size(), CV_32FC(img.channels())); 
        rng.fill(noise, cv::RNG::NORMAL, percentage_to_dispersion(img_depth, dispersion), percentage_to_offset(img_depth, offset));
        cv::Mat img_float;
        img.convertTo(img_float, CV_32F);
        img_float += noise;
        img_float.convertTo(img, img.type());
    }

    /**
    * @brief Modification contraste et luminosité d'une image
    *
    * @param img Image à modifier
    * @param contrast contrast = 100 : Pas de changement; contrast > 100 : Augmente le contraste; 0 < contrast < 100 : Réduit le contraste
    * @param bright bright = 0 : Neutre; bright > 0 : Éclaircit; bright < 0 : Assombrit
    *
    */

    void contrast_brightness_modifier(cv::Mat &img, int contrast, int bright)
    {
        if(contrast>0 && contrast<=100)
            contrast+=50;
        contrast = std::max(-100, std::min(100, contrast));
        bright = std::max(-100, std::min(100, bright));

        float alpha = 1.0f + (float(contrast) / 100.0f);  // [0.0, 2.0]
        float beta = float(bright) * 1.3f;                // [-130, 130]

        img.convertTo(img, -1, alpha, beta);
    }

    void ajouterTaches(cv::Mat& image, cv::RNG rng, int nombreTaches, int rayonMin, int rayonMax) {
        for(int i = 0; i < nombreTaches; i++) {
            cv::Point centre(rng.uniform(rayonMax, image.cols - rayonMax), rng.uniform(rayonMax, image.rows - rayonMax));
            int rayon = rng.uniform(rayonMin, rayonMax);
            cv::Scalar color = cv::Scalar(1, 1, 1);
            cv::circle(image, centre, rayon, color, -1);
        }
    }

    

    void rotate_img(int deg)
    {
        cv::Mat img_out = img.clone();
        cv::Mat identity = cv::Mat::eye(3, 3, CV_32F);
        identity *= rotate_center(deg, img_out.cols / 2, img_out.rows / 2);
        identity = identity(cv::Rect(0, 0, 3, 2));
        cv::warpAffine(img_out, img, identity, img.size(), cv::INTER_LINEAR);
    }

    void translate_img(int dx, int dy)
    {
        cv::Mat img_out = img.clone();
        cv::Mat affine = (cv::Mat_<float>(2,3) << 1, 0, dx, 0, 1, dy);
        cv::warpAffine(img_out, img, affine, img.size(), cv::INTER_LINEAR);
    }
    

    void random_exec()
    {
        cv::RNG rng;
        if(seed)
            rng= cv::RNG(seed);
        else
            rng = cv::RNG(time(0));

        rotate_img(rng.uniform(-5,5));
        translate_img(rng.uniform(-5,5), rng.uniform(-5,5));
        add_salt_pepper_noise(img, rng, rng.uniform(0.01,0.13), rng.uniform(0.01,0.13));
        add_gaussian_noise(img, rng, rng.uniform(1.0,5.0), rng.uniform(1.0,5.0));
        contrast_brightness_modifier(img, rng.uniform(-20,20), rng.uniform(-20,20));
        ajouterTaches(img, rng, rng.uniform(0,8), rng.uniform(4,8), rng.uniform(9,35));
    }

    void gestion_arg(int argc, char const* argv[])
    {   
        // juste sous votre prototype de gestion_arg()
        static const std::vector<std::string> poss_opt = {
            "-s=", "-g=", "-cb=", "-sp=", "-r=", "-t=", "-nb="
        };
        if (argc < 2)
        {
            std::cerr << "call -> usage() -> exit()" << std::endl;        
            exit (1);
        }
        if(argc==2)
        {
            random_exec();
            return;
        }
            
        //std::string poss_opt[]={"-s=", "-g=", "-cb=", "-sp=", "-r=", "-t=", "-nb="};
        for(int i=2; i<argc; i++)
        {
            std::string arg(argv[i]);
            size_t pos = arg.find("-seed=");
            if (pos != std::string::npos)
            {
                std::string str_seed_val = arg.substr(pos+6);
                if (str_seed_val.empty()) 
                {
                    std::cerr << "Erreur : la valeur de -seed= est vide." << std::endl;
                } 
                else 
                {
                    try {
                        int value = std::stoi(str_seed_val);  
                        std::cout << "Valeur de -seed : " << value << std::endl;
                        seed=value;
                    } catch (const std::invalid_argument& e) {
                        std::cerr << "Erreur : valeur non numérique pour -seed : '" << str_seed_val << "'" << std::endl;
                        //usage()
                    } catch (const std::out_of_range& e) {
                        std::cerr << "Erreur : valeur trop grande pour -seed : '" << str_seed_val << "'" << std::endl;
                        //usage()
                }
            }
        }
        std::map<std::string, std::string> parsed_opts;
        for(int i = 2; i < argc; ++i) {
            std::string arg(argv[i]);
            for(const auto &opt : poss_opt) {
                // Vérifie que l'argument commence par opt
                if (arg.rfind(opt, 0) == 0) {
                    // Extrait la valeur après le '='
                    std::string value = arg.substr(opt.size());
                    // Stocke en mappant l'option à sa valeur
                    parsed_opts[opt] = value;
                    break;  // passe à l'argument suivant
                }   
            }
        }
        auto it = parsed_opts.find("-r=");
        if (it != parsed_opts.end()) {
            int angle = std::stof(it->second);  // conversion sans surcoût
            rotate_img(angle);
    }
        //si seed not in params
    }
    }

    


    int main(int argc, char const* argv[])
    {
        if (argc < 2)
        {
            std::cerr << "Usage: " << argv[0] << " <image_max_pepperth>" << std::endl;
            return 1;
        }
        std::string image_max_pepperth = argv[1];
        img=cv::imread(image_max_pepperth);
        cv::Mat calibrated_img = img.clone();
        cv::Mat identity = cv::Mat::eye(3, 3, CV_32F);
        //  identity *= rotate_center(5, img.cols / 2, img.rows / 2);
        //print_mat(identity);
        //identity *= translate(3, 0);
        //print_mat(identity);
        //identity = identity(cv::Rect(0, 0, 3, 2));
        //print_mat(identity);
        noisy_img = img.clone();
        // ajouterTaches(noisy_img);  // Ajoute le bruit
        // contrast_brightness_modifier(noisy_img,60, 40);                          //SEG_FAULT
        // add_gaussian_noise(noisy_img, 20, 20);
        //add_salt_pepper_noise(noisy_img, 1, 10);
        //cv::warpAffine(noisy_img, calibrated_img, identity, noisy_img.size(), cv::INTER_LINEAR);
        
        gestion_arg(argc, argv);
        
        cv::imwrite("calibrated_img.png",img);

        

        return 0;
    }