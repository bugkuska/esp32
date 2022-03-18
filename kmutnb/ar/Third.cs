using System.Collections;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Networking;
using Vuforia;

public class Third : MonoBehaviour
{
    InputField MTemp;
    InputField MHum;
    InputField DTemp;
    InputField DHum;
    InputField ASoil;
    InputField Date;
    InputField Time;   

    public VirtualButtonBehaviour Vb_on;

    void Start()
    {
        MTemp = GameObject.Find("InputFieldMTemp").GetComponent<InputField>();
        MHum = GameObject.Find("InputFieldMHum").GetComponent<InputField>();
        DTemp = GameObject.Find("InputFieldDTemp").GetComponent<InputField>();
        DHum = GameObject.Find("InputFieldDHum").GetComponent<InputField>();
        ASoil = GameObject.Find("InputFieldASoil").GetComponent<InputField>();
        Time = GameObject.Find("InputFieldTime").GetComponent<InputField>();
        Date = GameObject.Find("InputFieldDate").GetComponent<InputField>();
       
        Vb_on.RegisterOnButtonPressed(OnButtonPressed_on);
        // GameObject.Find("GetButton").GetComponent<Button>().onClick.AddListener(GetData);
    }

    public void OnButtonPressed_on(VirtualButtonBehaviour Vb_on)
    {
        GetData_Mtem();
        GetData_Mhum();
        GetData_DTemp();
        GetData_DHum();
        GetData_ASoil();
        GetData_Time();
        GetData_Date();
        Debug.Log("Click");
    }

    void GetData_Mtem() => StartCoroutine(GetData_Coroutine1());
    void GetData_Mhum() => StartCoroutine(GetData_Coroutine2());
    void GetData_DTemp() => StartCoroutine(GetData_Coroutine3());
    void GetData_DHum() => StartCoroutine(GetData_Coroutine4());
    void GetData_ASoil() => StartCoroutine(GetData_Coroutine5());
    void GetData_Date() => StartCoroutine(GetData_Coroutine6());
    void GetData_Time() => StartCoroutine(GetData_Coroutine7());
 
    IEnumerator GetData_Coroutine1()
    {
        Debug.Log("Getting Data");
        MTemp.text = "Loading...";
        string uri = "https://sgp1.blynk.cloud/external/api/get?token=blynk_token&v?";
        using (UnityWebRequest request = UnityWebRequest.Get(uri))
        {
            yield return request.SendWebRequest();
            if (request.isNetworkError || request.isHttpError)
                MTemp.text = request.error;
            else
            {
                MTemp.text = request.downloadHandler.text;
            }
        }
    }
    IEnumerator GetData_Coroutine2()
    {
        Debug.Log("Getting Data");
        MHum.text = "Loading...";
        string uri = "https://sgp1.blynk.cloud/external/api/get?token=blynk_token&v?";
        using (UnityWebRequest request = UnityWebRequest.Get(uri))
        {
            yield return request.SendWebRequest();
            if (request.isNetworkError || request.isHttpError)
                MHum.text = request.error;
            else
            {
                MHum.text = request.downloadHandler.text;
            }
        }
    }
    IEnumerator GetData_Coroutine3()
    {
        Debug.Log("Getting Data");
        DTemp.text = "Loading...";
        string uri = "https://sgp1.blynk.cloud/external/api/get?token=blynk_token&v?";
        using (UnityWebRequest request = UnityWebRequest.Get(uri))
        {
            yield return request.SendWebRequest();
            if (request.isNetworkError || request.isHttpError)
                DTemp.text = request.error;
            else
            {
                DTemp.text = request.downloadHandler.text;
            }
        }
    }

    IEnumerator GetData_Coroutine4()
    {
        Debug.Log("Getting Data");
        DHum.text = "Loading...";
        string uri = "https://sgp1.blynk.cloud/external/api/get?token=blynk_token&v?";
        using (UnityWebRequest request = UnityWebRequest.Get(uri))
        {
            yield return request.SendWebRequest();
            if (request.isNetworkError || request.isHttpError)
                DHum.text = request.error;
            else
            {
                DHum.text = request.downloadHandler.text;
            }
        }
    }

    IEnumerator GetData_Coroutine5()
    {
        Debug.Log("Getting Data");
        ASoil.text = "Loading...";
        string uri = "https://sgp1.blynk.cloud/external/api/get?token=blynk_token&v?";
        using (UnityWebRequest request = UnityWebRequest.Get(uri))
        {
            yield return request.SendWebRequest();
            if (request.isNetworkError || request.isHttpError)
                ASoil.text = request.error;
            else
            {
                ASoil.text = request.downloadHandler.text;
            }
        }
    }

    IEnumerator GetData_Coroutine6()
    {
        Debug.Log("Getting Data");
        Time.text = "Loading...";
        string uri = "https://sgp1.blynk.cloud/external/api/get?token=blynk_token&v?";
        using (UnityWebRequest request = UnityWebRequest.Get(uri))
        {
            yield return request.SendWebRequest();
            if (request.isNetworkError || request.isHttpError)
                Time.text = request.error;
            else
            {
                Time.text = request.downloadHandler.text;
            }
        }
    }
    IEnumerator GetData_Coroutine7()
    {
        Debug.Log("Getting Data");
        Date.text = "Loading...";
        string uri = "https://sgp1.blynk.cloud/external/api/get?token=blynk_token&v?";
        using (UnityWebRequest request = UnityWebRequest.Get(uri))
        {
            yield return request.SendWebRequest();
            if (request.isNetworkError || request.isHttpError)
                Date.text = request.error;
            else
            {
                Date.text = request.downloadHandler.text;
            }
        }
    }
}
