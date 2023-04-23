


let selectedTab = 0;
let tabArr;
let canSwitch;
document.addEventListener('DOMContentLoaded', function() {
    tabArr = document.querySelectorAll('nav td');
    canSwitch=1;
});

onscroll = checkUpdateScroll;
function checkUpdateScroll(scrollPos)
{
    if(tabArr.length==null || canSwitch==0)
        return;

    let y = window.scrollY;
    let upOffset = 200;
    let membersDivPos = document.getElementById("members-div").offsetTop -upOffset;
    let docsDivPos = document.getElementById("docs-div").offsetTop -upOffset;;
    let vidsDivPos =document.getElementById("videos-div").offsetTop -upOffset;;
    let imagesDivPos =document.getElementById("images-div").offsetTop -upOffset;;
    let aboutDivPos =document.getElementById("about-div").offsetTop -upOffset;;


    if (y > membersDivPos)
    {

        tabArr[selectedTab].classList.remove("selected");
        tabArr[4].classList.add("selected");
        selectedTab = 4;
        canSwitch=0;
        setTimeout(haha, 100);
        return;
    }
    else if (y> docsDivPos && y<membersDivPos) 
    {

        tabArr[selectedTab].classList.remove("selected");
        tabArr[3].classList.add("selected");
        selectedTab = 3;
        canSwitch=0;
        setTimeout(haha, 100);
        return;
    }
    else if (y> vidsDivPos && y<docsDivPos) 
    {

        tabArr[selectedTab].classList.remove("selected");
        tabArr[2].classList.add("selected");
        selectedTab = 2;
        canSwitch=0;
        setTimeout(haha, 100);
        return;
    }
    else if (y> imagesDivPos && y<vidsDivPos) 
    {

        tabArr[selectedTab].classList.remove("selected");
        tabArr[1].classList.add("selected");
        selectedTab = 1;
        canSwitch=0;
        setTimeout(haha, 100);
        return;
    }
    else if(y> aboutDivPos)
    {
        tabArr[selectedTab].classList.remove("selected");
        tabArr[0].classList.add("selected");
        selectedTab = 0;
        canSwitch=0;
        setTimeout(haha, 100);
    }
    
}

function haha()
{
    canSwitch=1;
}