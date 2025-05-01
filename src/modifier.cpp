    #include <iostream>
    #include <common.h>
    #include <cstdlib>
    #include <string.h>
    #include <random>
    #include <tuple>
    #include "utils/math_utils.h"

    int img_depth;
    int seed=0;
    cv::Mat img;
    cv::Mat noisy_img;
    int nb_appl=0;

    /**
    * @brief Ajout de bruit poivre et sel
    *
    * @param img Image à modifier                                                                                  //TODO
    * @param max_pepper Pourcentage maximum de poivre
    * @param bright bright = 0 : Neutre; bright > 0 : Éclaircit; bright < 0 : Assombrit
    *
    */
    std::optional<std::tuple<int,int>> parse_sp(const std::string& value) {
        auto comma = value.find(',');
        if (comma == std::string::npos) return std::nullopt;
    
         try {
            int salt   = std::stoi(value.substr(0, comma));
            int pepper = std::stoi(value.substr(comma + 1));
            return std::make_tuple(salt, pepper);
        } catch (...) {
            return std::nullopt;
        }
    }

    std::optional<std::tuple<int,int,int>> parse_3(const std::string& s) {
        // On cherche deux virgules
        size_t p1 = s.find(',');
        if (p1 == std::string::npos) return std::nullopt;
        size_t p2 = s.find(',', p1 + 1);
        if (p2 == std::string::npos) return std::nullopt;
    
        try {
            int nb_spot    = std::stoi(s.substr(0,  p1));
            int min_radius = std::stoi(s.substr(p1 + 1, p2 - p1 - 1));
            int max_radius = std::stoi(s.substr(p2 + 1));
            return std::make_tuple(nb_spot, min_radius, max_radius);
        } catch (...) {
            return std::nullopt;
        }
    }

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
        //CAS TTES TRANSFORMATIONS FULL ALEATOIRE
        if(argc==2){
            random_exec();
            return;
        }
        //CAS TTES TRANSFORMATIONS SEED ALEATOIRES
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

        //CAS UNE OU PLUSIEURS TRANSFORMATIONS AVEC PARAM
        cv::RNG rng(time(0));
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
        
        if (auto it = parsed_opts.find("-sp="); it != parsed_opts.end()) {
            if (auto res = parse_sp(it->second)) {
                // Structured binding pour décomposer le tuple
                auto [salt, pepper] = *res;
                add_salt_pepper_noise(img, rng, pepper, salt);
            } else {
                std::cerr << "Erreur dans le format de -sp= (attendu: salt, pepper)\n";
                exit(1);
            }
        }
        if (auto it = parsed_opts.find("-g="); it != parsed_opts.end()) {
            if (auto res = parse_sp(it->second)) {
                // Structured binding pour décomposer le tuple
                auto [offset, dispersion] = *res;
                add_gaussian_noise(img, rng, offset, dispersion);
            } else {
                std::cerr << "Erreur dans le format de -g= (attendu: offset, dispersion)\n";
                exit(1);
            }
            }
            if (auto it = parsed_opts.find("-cb="); it != parsed_opts.end()) {
                if (auto res = parse_sp(it->second)) {
                    // Structured binding pour décomposer le tuple
                    auto [contrast, bright] = *res;
                    contrast_brightness_modifier(img, contrast, bright);
                } else {
                    std::cerr << "Erreur dans le format de -cb= (attendu: contrast, bright)\n";
                    exit(1);
                }

            }
            
            if (auto it = parsed_opts.find("-s="); it != parsed_opts.end()) {
                if (auto res = parse_3(it->second)) {
                    // Structured binding pour décomposer le tuple
                    auto [nb_spot, min_radius, max_radius] = *res;
                    ajouterTaches(img, rng, nb_spot, min_radius, max_radius);
                } else {
                    std::cerr << "Erreur dans le format de -s= (attendu: nb spot, min radius, max radius)\n";
                    exit(1);
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

        
        std::cout<<nb_appl<<std::endl;
        return 0;
    }