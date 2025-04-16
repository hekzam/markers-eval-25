#include <iostream>
#include <common.h>
#include <cstdlib>
#include "utils/math_utils.h"

int img_depth;
int seed=0;

/**
 * @brief Ajout de bruit poivre et sel
 *
 * @param img Image à modifier                                                                                  //TODO
 * @param max_pepper Pourcentage maximum de poivre
 * @param bright bright = 0 : Neutre; bright > 0 : Éclaircit; bright < 0 : Assombrit
 *
 */


void add_salt_pepper_noise(cv::Mat &img, int max_pepper, int max_salt)      //chevauchements
{   
    cv::RNG rng; 
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

void add_gaussian_noise(cv::Mat &img, int dispersion, int offset) {
    cv::Mat noise = cv::Mat::zeros(img.size(), CV_32FC(img.channels())); 
    cv::RNG rng(static_cast<unsigned int>(cv::getTickCount()));
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
    // Clamp les valeurs pour éviter les dépassements
    if(contrast>0 && contrast<=100)
        contrast+=50;
    contrast = std::max(-100, std::min(100, contrast));
    bright = std::max(-100, std::min(100, bright));

    float alpha = 1.0f + (float(contrast) / 100.0f);  // [0.0, 2.0]
    float beta = float(bright) * 1.3f;                // [-130, 130]

    img.convertTo(img, -1, alpha, beta);
}

void ajouterTaches(cv::Mat& image, int nombreTaches = 10, int rayonMin = 5, int rayonMax = 20) {
    cv::RNG rng(cv::getTickCount()); // Générateur aléatoire

    for(int i = 0; i < nombreTaches; i++) {
        cv::Point centre(rng.uniform(rayonMax, image.cols - rayonMax), rng.uniform(rayonMax, image.rows - rayonMax));
        int rayon = rng.uniform(rayonMin, rayonMax);
        cv::Scalar color = cv::Scalar(1, 1, 1);
        
        
        cv::circle(image, centre, rayon, color, -1);
    }
}

//si aleatoire pur  : Initialisation avec l'horloge système = srand(time(0));

int main(int argc, char const* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <image_max_pepperth>" << std::endl;
        return 1;
    }
    std::string image_max_pepperth = argv[1];
    cv::Mat img = cv::imread(image_max_pepperth);
    img_depth = img.depth();
    cv::Mat calibrated_img = img.clone();
    cv::Mat identity = cv::Mat::eye(3, 3, CV_32F);
    // identity *= rotate_center(5, img.cols / 2, img.rows / 2);
    //print_mat(identity);
    // identity *= translate(3, 0);
    //print_mat(identity);
    identity = identity(cv::Rect(0, 0, 3, 2));
    //print_mat(identity);
    cv::Mat noisy_img = img.clone();
    // ajouterTaches(noisy_img);  // Ajoute le bruit
    //contrast_brightness_modifier(noisy_img,atoi(argv[2]),atoi(argv[3]));
    //add_gaussian_noise(noisy_img, 20, 20);
    //  add_salt_pepper_noise(noisy_img, 1, 10);
    cv::warpAffine(noisy_img, calibrated_img, identity, noisy_img.size(), cv::INTER_LINEAR);
    cv::imwrite("calibrated_img.png",calibrated_img);

    return 0;
}